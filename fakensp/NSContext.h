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
	DWORD Flags;		//��¼NSPLookupServiceBegin���inFlags������ֻ��NSPLookupServiceNext��ʹ��
	std::wstring Name;	//����

	DWORD Deep;			//��¼����NSPLookupServiceNext����ȣ�ÿ��+1
	BLOB* Blob;			//��¼���ص�Blob,����NSPLookupServiceEnd���ͷ�
	CSADDR_INFO* AddrInfo;	//��¼���ص�AddrInfo������NSPLookupServiceEnd���ͷ�
};

