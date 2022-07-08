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
	TRACEX_( L"[fakensp] NSPLookupServiceBegin() begin flag: %x\n", inFlags);

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

		DWORD family = 0;
		if (IsEqualGUID(*inQuerySet->lpServiceClassId, SVCID_INET_HOSTADDRBYNAME))
		{
			family = AF_INET;
		}
		else if (IsEqualGUID(*inQuerySet->lpServiceClassId, SVCID_DNS_TYPE_A))
		{
			family = AF_INET;
		}
		else if (IsEqualGUID(*inQuerySet->lpServiceClassId, SVCID_DNS_TYPE_AAAA))
		{
			family = AF_INET6;
		}

		if (family == 0)
		{
			err = WSAEINVAL; break;
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

		ctx->Family = family;
		ctx->Flags = inFlags;
		ctx->Name = name;

		//Proxifier�ķ�������ֹ����ϵͳ��NSP����Ϊϵͳ�����dwNameSpace��
		//����������NSP����û�м��dwNameSpace������Proxifierû�м�飩�����»��ǻ������������NSP��
		inQuerySet->dwNameSpace = NS_TCPIP_HOSTS;

		*outLookup = (HANDLE)ctx;

		err = NO_ERROR;
	} while (false);

	TRACEX_(L"[fakensp] NSPLookupServiceBegin end err: %d\n", err);

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
	TRACEX_(L"[fakensp] NSPLookupServiceNext() begin inFlags: %x\n", inFlags);
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

		memset(outResults, 0, sizeof(*outResults));

		NSContext* ctx = (NSContext*)inLookup;

		TRACEX_(L"[fakensp] NSPLookupServiceNext() ctx: %p | family: %d\n", ctx, ctx->Family);


		DWORD deep = InterlockedIncrement(&ctx->Deep);
		if (deep == 2)
		{
			//�ڶ��ε���ʱ����NO_MORE��ֹ��������Next
			err = WSA_E_NO_MORE;
			//���NSPProvider�б���ֹ�Ժ�����NSP����(����ϵͳNSP������NSP)
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

		_ASSERT(ctx->Family == AF_INET || ctx->Family == AF_INET6);

		DWORD error_now = 0;
		DWORD addrlen = ctx->Family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
		sockaddr* addr = (sockaddr*)alloca(addrlen); memset(addr, 0, addrlen);
		addr->sa_family = (WORD)ctx->Family;

		if (ctx->Family == AF_INET)
		{
			TRACEX_(L"[fakensp] NSPLookupServiceNext() A\n");
			sockaddr_in* addr4 = (sockaddr_in*)addr;
			if (!gIPMapping.v4GeneralAndAdd(ctx->Name, 20, addr4->sin_addr))
			{
				//û�п��õ�IP
				err = WSA_NOT_ENOUGH_MEMORY;
				break;
			}
		}else
		{
			TRACEX_(L"[fakensp] NSPLookupServiceNext() AAAA\n");

			sockaddr_in6* addr6 = (sockaddr_in6*)addr;
			if (!gIPMapping.v6GeneralAndAdd(ctx->Name, 20, addr6->sin6_addr))
			{
				//û�п��õ�IP
				err = WSA_NOT_ENOUGH_MEMORY;
				break;
			}
			addr6->sin6_addr.u.Byte[0] = 0x11;
			addr6->sin6_addr.u.Byte[1] = 0x22;
			addr6->sin6_addr.u.Byte[2] = 0x33;
			addr6->sin6_addr.u.Byte[3] = 0x44;
			addr6->sin6_addr.u.Byte[4] = 0x55;
			addr6->sin6_addr.u.Byte[5] = 0x66;
			addr6->sin6_addr.u.Byte[6] = 0x77;
			addr6->sin6_addr.u.Byte[7] = 0x88;
			addr6->sin6_addr.u.Byte[8] = 0x99;
			addr6->sin6_addr.u.Byte[9] = 0x11;
			addr6->sin6_addr.u.Byte[10] = 0x22;
			addr6->sin6_addr.u.Byte[11] = 0x33;
			addr6->sin6_addr.u.Byte[12] = 0x44;
			addr6->sin6_addr.u.Byte[13] = 0x55;
			addr6->sin6_addr.u.Byte[14] = 0x66;
			addr6->sin6_addr.u.Byte[15] = 0x77;
		}

		if (ctx->Flags & LUP_API_ANSI)
		{
			TRACEX_(L"[fakensp] NSPLookupServiceNext() LUP_API_ANSI\n");
			//�õ�getaddrinfoA
		}
		else 
		{
			//�õ�getaddrinfoW
		}

		if (ctx->Flags & LUP_RETURN_NAME)
		{
			TRACEX_(L"[fakensp] NSPLookupServiceNext() LUP_RETURN_NAME\n");

			//������������������
			outResults->lpszServiceInstanceName =(WCHAR*)ctx->Name.c_str();
		}

		if (ctx->Flags & LUP_RETURN_BLOB)
		{
			TRACEX_(L"[fakensp] NSPLookupServiceNext() LUP_RETURN_BLOB\n");

			//��Ҫ��gethostbyname��Ҫ�󷵻�hostent��blob�ṹ
			std::string nameA = UtilString::w2s(ctx->Name);

			BLOB* blob = Util::PackHostEntBlob(nameA, ctx->Family, addr, 1);
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
			TRACEX_(L"[fakensp] NSPLookupServiceNext() LUP_RETURN_ADDR\n");

			//��Ҫ��getaddrinfo��Ҫ�󷵻�CSADDR_INFO�Ľṹ����BLOB
			CSADDR_INFO* addrinfo = new (std::nothrow)CSADDR_INFO;
			sockaddr* remote = (sockaddr*)new (std::nothrow)char[addrlen];
			if (addrinfo == NULL && remote == NULL)
			{
				if (addrinfo != NULL)
					delete addrinfo;
				if (remote != NULL)
					delete remote;
				err = WSA_NOT_ENOUGH_MEMORY;
				break;
			}

			memset(addrinfo, 0, sizeof(*addrinfo));
			memcpy(remote, addr, addrlen);

			addrinfo->iSocketType = SOCK_DGRAM;
			addrinfo->iProtocol = IPPROTO_UDP;
			addrinfo->LocalAddr.iSockaddrLength = 0;
			addrinfo->LocalAddr.lpSockaddr = NULL;
			addrinfo->RemoteAddr.iSockaddrLength = addrlen;
			addrinfo->RemoteAddr.lpSockaddr = remote;
			
			outResults->dwNumberOfCsAddrs = 1;
			outResults->lpcsaBuffer = addrinfo;
			outResults->dwOutputFlags = RESULT_IS_ADDED;
			ctx->AddrInfo = addrinfo;
		}

	} while (false);

	TRACEX_(L"[fakensp] NSPLookupServiceNext() end err: %d\n", err);

	if (err != NO_ERROR)
	{
		SetLastError(err);
		return SOCKET_ERROR;
	}

	return NO_ERROR;
}
int WSPAPI NSPLookupServiceEnd(HANDLE inLookup)
{
	TRACEX_(L"[fakensp] NSPLookupServiceEnd()\n");

	do
	{
		if (inLookup == NULL)
			break;

		NSContext* ctx = (NSContext*)inLookup;
		delete ctx;

	} while (false);

	return NO_ERROR;
}
/////////////////////////////////////////////////////////////////////////////////

int WSPAPI NSPCleanup(LPGUID lpProviderId)
{
	TRACEX_(L"[fakensp] NSPCleanup()\n");
	return NO_ERROR;
}
int WSPAPI NSPStartup(LPGUID inProviderID, LPNSP_ROUTINE outRoutines)
{
	TRACEX_(L"[fakensp] NSPStartup()\n");

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