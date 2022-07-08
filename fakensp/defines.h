#pragma once

#define LOG_TYPE							1

#define LUP_DISABLE_IDN_ENCODING    0x00800000
#define LUP_API_ANSI				0x01000000		//存在时，表示用的getaddrinfoA

#define LUP_RESOLUTION_HANDLE		0x80000000

typedef struct _NSPROVIDER
{
	LONG RefCount;
	DWORD NamespaceId;
	HINSTANCE DllHandle;
	GUID ProviderId;
	NSP_ROUTINE Service;
} NSPROVIDER, *PNS_PROVIDER;

typedef struct _NSQUERY_PROVIDER
{
	LIST_ENTRY QueryLink;
	PNS_PROVIDER Provider;
	HANDLE LookupHandle;
} NSQUERY_PROVIDER, *PNSQUERY_PROVIDER;

typedef struct _NSQUERY
{
	DWORD Signature;    //0xBEADFACE
	LONG RefCount;
	BOOLEAN ShuttingDown;
	LIST_ENTRY ProviderList;
	PNSQUERY_PROVIDER ActiveProvider;
	RTL_CRITICAL_SECTION Lock;
	PNSQUERY_PROVIDER CurrentProvider;
	LPWSAQUERYSETW QuerySet;
	DWORD ControlFlags;
	void* Catalog;      //PNSCATALOG Catalog;
	DWORD TryAgain;
} NSQUERY, *PNSQUERY;


struct IN_ADDR_CMPARE
{
	bool operator ()(const IN_ADDR& c1, const IN_ADDR& c2) const
	{
		return c1.S_un.S_addr < c2.S_un.S_addr;
	}
};

struct IN6_ADDR_CMPARE
{
	bool operator ()(const IN6_ADDR& c1, const IN6_ADDR& c2) const
	{
		uint32_t* p1 = (uint32_t*)c1.u.Byte;
		uint32_t* p2 = (uint32_t*)c2.u.Byte;

		if (p1[0] < p2[0] ||
			p1[1] < p2[1] ||
			p1[2] < p2[2] ||
			p1[3] < p2[3])
		{
			return true;
		}
		return false;
	}
};