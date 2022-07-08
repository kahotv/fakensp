#include "stdafx.h"

bool gInit = false;
std::mutex gInitMtx;

IPMapping gIPMapping;

/////////////////////////////////////////////////////////////////////////////////
//Service相关功能用不到，直接返回不支持

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
//NS_DNS要用到的功能

//开始检索，失败时返回特定的错误码
//https://docs.microsoft.com/zh-cn/windows/win32/api/ws2spi/nc-ws2spi-lpnsplookupservicebegin
/*
WSA_NOT_ENOUGH_MEMORY		内存资源不足，无法处理此命令。
WSAEINVAL					提供了一个无效的参数。
WSAEOPNOTSUPP				参考的对象类型不支持尝试的操作。
WSANO_DATA					请求的名称有效，但是找不到请求的类型的数据。
WSASERVICE_NOT_FOUND		此服务不存在。在指定的命名空间中找不这个服务。
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
		//第3~9次不生效，测试由此测试移除ActiveProvider对系统造成的影响 （结论是不影响）
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
			//我们只支持NS_DNS
			err = WSASERVICE_NOT_FOUND; break;
		}
		if (inQuerySet->lpszServiceInstanceName == NULL)
		{
			err = WSAEINVAL; break;
		}
		if ((inFlags & (LUP_RETURN_ADDR | LUP_RETURN_BLOB)) == 0)
		{
			//如果不要求返回CSADDR_INFO或者HOSTENT，就返回不支持
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
			//放行特殊域名
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

		//限制进程
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

		//Proxifier的方法，阻止调用系统的NSP，因为系统检查了dwNameSpace。
		//但其他三方NSP可能没有检查dwNameSpace（至少Proxifier没有检查），导致还是会继续调用三方NSP。
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

//可选的错误
//https://docs.microsoft.com/zh-cn/windows/win32/api/ws2spi/nc-ws2spi-lpnsplookupservicenext
/*
WSA_E_CANCELLED			10111		在该调用还处于处理中时，就调用了 WSALookupServiceEnd。该调用被取消。
WSA_E_NO_MORE			10110		WSALookupServiceNext 不能返回更多的结果。
WSA_INVALID_HANDLE		6			句柄无效。
WSA_NOT_ENOUGH_MEMORY	8			内存资源不足，无法处理此命令。
WSAEFAULT				10014		系统检测到在一个调用中尝试使用指针参数时的无效指针地址。
WSAEINVAL				10022		提供了一个无效的参数。
WSAEOPNOTSUPP			10045		参考的对象类型不支持尝试的操作。
WSANO_DATA				11004		请求的名称有效，但是找不到请求的类型的数据。
WSASERVICE_NOT_FOUND	10108		此服务不存在。在指定的命名空间中找不这个服务。
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
			//内存不足，告诉外层我们需要的最小内存
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
			//第二次调用时返回NO_MORE阻止继续调用Next
			err = WSA_E_NO_MORE;
			//清空NSPProvider列表，阻止对后续的NSP调用(包括系统NSP和三方NSP)
#ifdef _WIN64
			NSQUERY* pNsQuery = Util::SpiScanNsQuery(0x100);
#else
			NSQUERY* pNsQuery = Util::SpiScanNsQuery(5);
#endif
			if (pNsQuery != NULL)
			{
				//XP下若是不执行这一句，会造成自身的NSP结果被丢弃，其他系统会造成结果被排序到系统NSP之后
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
				//没有可用的IP
				err = WSA_NOT_ENOUGH_MEMORY;
				break;
			}
		}else
		{
			TRACEX_(L"[fakensp] NSPLookupServiceNext() AAAA\n");

			sockaddr_in6* addr6 = (sockaddr_in6*)addr;
			if (!gIPMapping.v6GeneralAndAdd(ctx->Name, 20, addr6->sin6_addr))
			{
				//没有可用的IP
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
			//用的getaddrinfoA
		}
		else 
		{
			//用的getaddrinfoW
		}

		if (ctx->Flags & LUP_RETURN_NAME)
		{
			TRACEX_(L"[fakensp] NSPLookupServiceNext() LUP_RETURN_NAME\n");

			//返回域名，这个必须的
			outResults->lpszServiceInstanceName =(WCHAR*)ctx->Name.c_str();
		}

		if (ctx->Flags & LUP_RETURN_BLOB)
		{
			TRACEX_(L"[fakensp] NSPLookupServiceNext() LUP_RETURN_BLOB\n");

			//主要是gethostbyname，要求返回hostent的blob结构
			std::string nameA = UtilString::w2s(ctx->Name);

			BLOB* blob = Util::PackHostEntBlob(nameA, ctx->Family, addr, 1);
			if (blob == NULL)
			{
				//内存不足
				err = WSA_NOT_ENOUGH_MEMORY;
				break;
			}

			//记录下来，要在End里释放
			ctx->Blob = blob;

			//返回
			outResults->lpBlob = blob;
		}

		if (ctx->Flags & LUP_RETURN_ADDR)
		{
			TRACEX_(L"[fakensp] NSPLookupServiceNext() LUP_RETURN_ADDR\n");

			//主要是getaddrinfo，要求返回CSADDR_INFO的结构，非BLOB
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

	//Service相关的函数必须设置，否则系统会认为NSPStartup调用失败
	outRoutines->NSPSetService = NSPSetService;
	outRoutines->NSPInstallServiceClass = NSPInstallServiceClass;
	outRoutines->NSPRemoveServiceClass = NSPRemoveServiceClass;
	outRoutines->NSPGetServiceClassInfo = NSPGetServiceClassInfo;
	outRoutines->NSPIoctl = NULL;		//不需要扩展功能

	return NO_ERROR;
}