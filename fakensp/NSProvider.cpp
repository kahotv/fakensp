#include "stdafx.h"

bool gInit = false;
std::mutex gInitMtx;

IPMapping gIPMapping;

/////////////////////////////////////////////////////////////////////////////////
//Service��ع����ò�����ֱ�ӷ��ز�֧��

int WSPAPI NSPSetService(LPGUID inProviderID, LPWSASERVICECLASSINFOW inServiceClassInfo, LPWSAQUERYSETW inRegInfo, WSAESETSERVICEOP inOperation, DWORD inFlags)
{
	SetLastError(WSASERVICE_NOT_FOUND);
	return SOCKET_ERROR;
}
int WSPAPI	NSPInstallServiceClass(LPGUID inProviderID, LPWSASERVICECLASSINFOW inServiceClassInfo)
{
	SetLastError(WSASERVICE_NOT_FOUND);
	return SOCKET_ERROR;
}
int WSPAPI	NSPRemoveServiceClass(LPGUID inProviderID, LPGUID inServiceClassID)
{
	SetLastError(WSASERVICE_NOT_FOUND);
	return SOCKET_ERROR;
}
int WSPAPI	NSPGetServiceClassInfo(LPGUID inProviderID, LPDWORD ioBufSize, LPWSASERVICECLASSINFOW ioServiceClassInfo)
{
	SetLastError(WSASERVICE_NOT_FOUND);
	return SOCKET_ERROR;
}
/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
//NS_DNSҪ�õ��Ĺ���

//��ʼ������ʧ��ʱ�����ض��Ĵ�����
//https://docs.microsoft.com/zh-cn/windows/win32/api/ws2spi/nc-ws2spi-lpnsplookupservicebegin
/*
WSA_NOT_ENOUGH_MEMORY		�ڴ���Դ���㣬�޷���������
WSAEINVAL					�ṩ��һ����Ч�Ĳ�����
WSAEOPNOTSUPP				�ο��Ķ������Ͳ�֧�ֳ��ԵĲ�����
WSANO_DATA					�����������Ч�������Ҳ�����������͵����ݡ�
WSASERVICE_NOT_FOUND		�˷��񲻴��ڡ���ָ���������ռ����Ҳ��������
*/
int WSPAPI NSPLookupServiceBegin(
	LPGUID					inProviderID,
	LPWSAQUERYSETW			inQuerySet,
	LPWSASERVICECLASSINFOW	inServiceClassInfo,
	DWORD					inFlags,
	LPHANDLE				outLookup)
{
	OutputDebugStringW(L"[mynsp] NSPLookupServiceBegin() begin\n");



	int err = WSASERVICE_NOT_FOUND;
	do
	{
		//��3~9�β���Ч�������ɴ˲����Ƴ�ActiveProvider��ϵͳ��ɵ�Ӱ�� �������ǲ�Ӱ�죩
		/*
		static ULONG i = 0;
		auto tmp = InterlockedIncrement(&i);
		if (tmp > 3 && tmp<9)
		{
			err = WSASERVICE_NOT_FOUND; break;
		}
		*/

		if (outLookup == NULL)
		{
			err = WSAEINVAL; break;
		}
		if (inQuerySet == NULL)
		{
			err = WSAEINVAL; break;
		}
		if (inQuerySet->dwNameSpace != NS_DNS && inQuerySet->dwNameSpace != NS_ALL)
		{
			//����ֻ֧��NS_DNS
			err = WSASERVICE_NOT_FOUND; break;
		}
		if (inQuerySet->lpszServiceInstanceName == NULL)
		{
			err = WSAEINVAL; break;
		}
		if ((inFlags & (LUP_RETURN_ADDR | LUP_RETURN_BLOB)) == 0)
		{
			//�����Ҫ�󷵻�CSADDR_INFO����HOSTENT���ͷ��ز�֧��
			err = WSASERVICE_NOT_FOUND; break;
		}

		std::wstring name = inQuerySet->lpszServiceInstanceName;
		if (UtilString::EndWith(name, L".local"))
		{
			//������������
			err = WSASERVICE_NOT_FOUND; break;
		}

		/*
		INT family = 0;
		INT protocol = 0;
		DWORD n = inQuerySet->dwNumberOfProtocols;
		if (n > 0) {
			if (inQuerySet->lpafpProtocols == NULL) {
				err = WSAEINVAL; break;
			}
			bool found = false;
			for (DWORD i = 0; i < n; i++) {
				family = inQuerySet->lpafpProtocols[i].iAddressFamily;
				protocol = inQuerySet->lpafpProtocols[i].iProtocol;
				if ((family == AF_INET) && ((protocol == IPPROTO_UDP) || (protocol == IPPROTO_TCP)))
				{
					found = true;
					break;
				}
			}
			if (!found) {
				err = WSASERVICE_NOT_FOUND; break;
			}
		}
		*/

		//���ƽ���
		std::wstring path = UtilString::ToLower(Util::GetCurrentExePath());
		if (!UtilString::EndWith(path, L"testnsp.exe") && name != L"www.baidu.com")
		{
			err = WSAEINVAL; break;
		}

		NSContext* ctx = new (std::nothrow) NSContext();
		if (ctx == NULL)
		{
			err = WSA_NOT_ENOUGH_MEMORY; break;
		}

		ctx->Flags = inFlags;
		ctx->Name = name;

		*outLookup = (HANDLE)ctx;

		err = NO_ERROR;
	} while (false);

	WCHAR buf2[0x200];
	wsprintfW(buf2, L"[mynsp] NSPLookupServiceBegin end err: %d\n", err);
	OutputDebugStringW(buf2);

	if (err != NO_ERROR)
	{
		SetLastError(err);
		return SOCKET_ERROR;
	}

	return NO_ERROR;

}

//��ѡ�Ĵ���
//https://docs.microsoft.com/zh-cn/windows/win32/api/ws2spi/nc-ws2spi-lpnsplookupservicenext
/*
WSA_E_CANCELLED			10111		�ڸõ��û����ڴ�����ʱ���͵����� WSALookupServiceEnd���õ��ñ�ȡ����
WSA_E_NO_MORE			10110		WSALookupServiceNext ���ܷ��ظ���Ľ����
WSA_INVALID_HANDLE		6			�����Ч��
WSA_NOT_ENOUGH_MEMORY	8			�ڴ���Դ���㣬�޷���������
WSAEFAULT				10014		ϵͳ��⵽��һ�������г���ʹ��ָ�����ʱ����Чָ���ַ��
WSAEINVAL				10022		�ṩ��һ����Ч�Ĳ�����
WSAEOPNOTSUPP			10045		�ο��Ķ������Ͳ�֧�ֳ��ԵĲ�����
WSANO_DATA				11004		�����������Ч�������Ҳ�����������͵����ݡ�
WSASERVICE_NOT_FOUND	10108		�˷��񲻴��ڡ���ָ���������ռ����Ҳ��������
*/
int WSPAPI
NSPLookupServiceNext(
	HANDLE			inLookup,
	DWORD			inFlags,
	LPDWORD			ioSize,
	LPWSAQUERYSETW	outResults)
{
	OutputDebugStringW(L"[mynsp] NSPLookupServiceNext() begin\n");
	int err = NO_ERROR;

	do
	{
		if (inLookup == NULL || ioSize == NULL || outResults == NULL)
		{
			err = WSAEINVAL; 
			break;
		}

		if (*ioSize < sizeof(WSAQUERYSETW))
		{
			//�ڴ治�㣬�������������Ҫ����С�ڴ�
			*ioSize = sizeof(WSAQUERYSETW);
			err = WSA_NOT_ENOUGH_MEMORY; 
			break;
		}
		OutputDebugStringW(L"[mynsp] NSPLookupServiceNext() ctx\n");

		memset(outResults, 0, sizeof(*outResults));

		NSContext* ctx = (NSContext*)inLookup;
		DWORD deep = InterlockedIncrement(&ctx->Deep);
		if (deep == 2)
		{
			//�ڶ��ε���ʱ����NO_MORE��ֹ��������Next
			err = WSA_E_NO_MORE;
			//���NSPProvider�б���ֹ��ϵͳ�ͺ�����NSP����
			//�������ֻ�����ȼ�������͵�NSP��Ч
#ifdef _WIN64
			NSQUERY* pNsQuery = Util::SpiScanNsQuery(0x100);
#else
			NSQUERY* pNsQuery = Util::SpiScanNsQuery(5);
#endif
			if (pNsQuery != NULL)
			{
				//XP�����ǲ�ִ����һ�䣬����������NSP���������������ϵͳ����ɽ��������ϵͳNSP֮��
				pNsQuery->ActiveProvider = NULL;
			}
			break;
		}

		DWORD error_now = 0;
		IN_ADDR ip = {};
		if (!gIPMapping.v4GeneralAndAdd(ctx->Name, 20, ip))
		{
			//û�п��õ�IP
			err = WSA_NOT_ENOUGH_MEMORY;
			break;
		}

		if (ctx->Flags & LUP_RETURN_NAME)
		{
			OutputDebugStringW(L"[mynsp] NSPLookupServiceNext() LUP_RETURN_NAME\n");

			//������������������
			outResults->lpszServiceInstanceName =(WCHAR*)ctx->Name.c_str();
		}

		if (ctx->Flags & LUP_RETURN_BLOB)
		{
			OutputDebugStringW(L"[mynsp] NSPLookupServiceNext() LUP_RETURN_BLOB\n");

			//��Ҫ��gethostbyname��Ҫ�󷵻�hostent��blob�ṹ
			std::string nameA = UtilString::w2s(ctx->Name);
			sockaddr_in v4 = {};
			v4.sin_addr = ip;

			BLOB* blob = Util::PackHostEntBlob(nameA, AF_INET, (const sockaddr*)&v4, 1);
			if (blob == NULL)
			{
				//�ڴ治��
				err = WSA_NOT_ENOUGH_MEMORY;
				break;
			}

			//��¼������Ҫ��End���ͷ�
			ctx->Blob = blob;

			//����
			outResults->lpBlob = blob;
		}

		if (ctx->Flags & LUP_RETURN_ADDR)
		{
			OutputDebugStringW(L"[mynsp] NSPLookupServiceNext() LUP_RETURN_ADDR\n");

			//��Ҫ��getaddrinfo��Ҫ�󷵻�CSADDR_INFO�Ľṹ����BLOB
			CSADDR_INFO* addr = new (std::nothrow)CSADDR_INFO;
			sockaddr_in* remote = new (std::nothrow)sockaddr_in;
			if (addr == NULL && remote == NULL)
			{
				if (addr != NULL)
					delete addr;
				if (remote != NULL)
					delete remote;
				err = WSA_NOT_ENOUGH_MEMORY;
				break;
			}

			memset(addr, 0, sizeof(*addr));
			memset(remote, 0, sizeof(*remote));

			remote->sin_family = AF_INET;
			remote->sin_addr = ip;
			addr->iSocketType = AF_INET;
			addr->iProtocol = IPPROTO_UDP;
			addr->LocalAddr.iSockaddrLength = 0;
			addr->LocalAddr.lpSockaddr = NULL;
			addr->RemoteAddr.iSockaddrLength = sizeof(*remote);
			addr->RemoteAddr.lpSockaddr = (struct sockaddr*)remote;
			
			outResults->dwNumberOfCsAddrs = 1;
			outResults->lpcsaBuffer = addr;

			ctx->AddrInfo = addr;
		}

	} while (false);

	WCHAR buf2[0x200];
	wsprintfW(buf2, L"[mynsp] NSPLookupServiceNext() end err: %d\n", err);
	OutputDebugStringW(buf2);

	if (err != NO_ERROR)
	{
		SetLastError(err);
		return SOCKET_ERROR;
	}

	return NO_ERROR;
}
int WSPAPI NSPLookupServiceEnd(HANDLE inLookup)
{
	OutputDebugStringW(L"[mynsp] NSPLookupServiceEnd()\n");

	do
	{
		if (inLookup == NULL)
			break;

		NSContext* ctx = (NSContext*)inLookup;
		ctx->Name = L"asdsada";
		delete ctx;

	} while (false);

	return NO_ERROR;
}
/////////////////////////////////////////////////////////////////////////////////

int WSPAPI NSPCleanup(LPGUID lpProviderId)
{
	OutputDebugStringW(L"[mynsp] NSPCleanup()\n");
	return NO_ERROR;
}
int WSPAPI NSPStartup(LPGUID inProviderID, LPNSP_ROUTINE outRoutines)
{
	OutputDebugStringW(L"[mynsp] NSPStartup()\n");

	memset(outRoutines, 0, sizeof(NSP_ROUTINE));

	outRoutines->cbSize = FIELD_OFFSET(NSP_ROUTINE, NSPIoctl);
	outRoutines->dwMajorVersion = 1;
	outRoutines->dwMinorVersion = 1;
	outRoutines->NSPCleanup = NSPCleanup;
	outRoutines->NSPLookupServiceBegin = NSPLookupServiceBegin;
	outRoutines->NSPLookupServiceNext = NSPLookupServiceNext;
	outRoutines->NSPLookupServiceEnd = NSPLookupServiceEnd;

	//Service��صĺ����������ã�����ϵͳ����ΪNSPStartup����ʧ��
	outRoutines->NSPSetService = NSPSetService;
	outRoutines->NSPInstallServiceClass = NSPInstallServiceClass;
	outRoutines->NSPRemoveServiceClass = NSPRemoveServiceClass;
	outRoutines->NSPGetServiceClassInfo = NSPGetServiceClassInfo;
	outRoutines->NSPIoctl = NULL;		//����Ҫ��չ����

	return NO_ERROR;
}