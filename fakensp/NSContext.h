#pragma once
class NSContext
{
public:
	NSContext() :Family(0), Flags(0), Deep(0), AddrInfo(NULL), Blob(NULL) {}
	~NSContext()
	{
		if (Blob != NULL)
		{
			delete Blob;
			Blob = NULL;
		}
		if (AddrInfo != NULL)
		{
			if (AddrInfo->LocalAddr.lpSockaddr != NULL)
			{
				delete AddrInfo->LocalAddr.lpSockaddr;
				AddrInfo->LocalAddr.lpSockaddr = NULL;
			}
			if (AddrInfo->RemoteAddr.lpSockaddr != NULL)
			{
				delete AddrInfo->RemoteAddr.lpSockaddr;
				AddrInfo->RemoteAddr.lpSockaddr = NULL;
			}
			delete AddrInfo;
			AddrInfo = NULL;
		}
	}


public:
	DWORD Family;		//AF_INET or AF_INET6
	DWORD Flags;		//记录NSPLookupServiceBegin里的inFlags参数，只在NSPLookupServiceNext里使用
	std::wstring Name;	//域名

	DWORD Deep;			//记录调用NSPLookupServiceNext的深度，每次+1
	BLOB* Blob;			//记录返回的Blob,用于NSPLookupServiceEnd里释放
	CSADDR_INFO* AddrInfo;	//记录返回的AddrInfo，用于NSPLookupServiceEnd里释放
};

