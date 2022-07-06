#include "stdafx.h"
#include "Util.h"


NSQUERY* Util::SpiScanNsQuery(DWORD deep)
{
	UINT_PTR tmpebp = 0;
#ifdef _WIN64
	tmpebp = (UINT_PTR)amd64_getRbp();
#else
	__asm mov tmpebp, ebp;
#endif
	while (deep-- > 0)
	{
		if (IsBadReadPtr((void*)tmpebp, sizeof(void*)))
			break;
		tmpebp = *(UINT_PTR*)tmpebp;
		if (IsBadReadPtr((void*)(tmpebp + 8), sizeof(void*)))
			continue;
		NSQUERY* pNsQuery = *(NSQUERY**)(tmpebp + 8);
		if (IsBadReadPtr(pNsQuery, sizeof(NSQUERY)))
			continue;
		//NSQUERY结构有个Signature，刚好可以用来做搜索。
		//在NT架构里，往上2~3层里的第一个参数就是PNSQUERY
		//https://github.com/mirror/reactos/blob/c6d2b35ffc91e09f50dfb214ea58237509329d6b/reactos/dll/win32/ws2_32_new/src/nsquery.c
		//line 47
		/*
		DWORD WSAAPI WsNqInitialize(IN PNSQUERY Query)
		{
			// Initialize the lock
			InitializeCriticalSection((LPCRITICAL_SECTION)&Query->Lock);

			// Set initial reference count and signature
			Query->RefCount = 1;
			Query->Signature = 0xBEADFACE;

			// Return success
			return ERROR_SUCCESS;
		}
		*/
		if (pNsQuery->Signature == 0xBEADFACE)
		{
			return pNsQuery;
		}
	}

	return NULL;
}

BLOB* Util::PackHostEntBlob(
	IN const std::string& hostname, 
	IN DWORD family, 
	IN const sockaddr* addrlist, 
	IN DWORD addrnum)
{
	BLOB* blob = NULL;
	short addrlen = family == AF_INET ? sizeof(IN_ADDR) : sizeof(IN6_ADDR);
	ULONG size = 0;

	auto dd = sizeof(ULONG);

	size += sizeof(BLOB);
	size += sizeof(hostent);
	size += (0 + 1) * RTL_FIELD_SIZE(hostent, h_aliases);			//Alias数组     hostent::h_aliases   void*
	size += (addrnum + 1) * RTL_FIELD_SIZE(hostent, h_addr_list);	//Address数组   hostent::h_addr_list void*
	size += addrnum * addrlen;										//Address 数据数组    IN_ADDR[] or IN6_ADDR[]
	size += (ULONG)hostname.length() + 1;									//HostName数据+结束符

	blob = (BLOB*)new (std::nothrow)char[size];
	if (blob != NULL)
	{
		memset(blob, 0, size);

		blob->cbSize = size - sizeof(BLOB);
		blob->pBlobData = (BYTE*)blob + sizeof(*blob);

		BYTE* head = blob->pBlobData;
		hostent* ent = (hostent*)head;
		BYTE* base = (BYTE*)(ent + 1);

		//Alias
		ent->h_aliases = (char**)(base);
		ent->h_aliases[0] = NULL;
		base += sizeof(ent->h_aliases[0]);

		//Address
		ent->h_addr_list = (char**)(base);
		ent->h_addr_list[addrnum] = NULL;       //最后一个设置为NULL
		base += (addrnum + 1) * sizeof(ent->h_addr_list[0]);
		for (ULONG i = 0; i < addrnum; i++)
		{
			ent->h_addr_list[i] = (char*)base + (i * addrlen);
			if (family == AF_INET)
			{
				sockaddr_in* addr = (sockaddr_in*)addrlist;
				memcpy(ent->h_addr_list[i], &addr[i].sin_addr, addrlen);
			}
			else if (family == AF_INET6)
			{
				sockaddr_in6* addr = (sockaddr_in6*)addrlist;
				memcpy(ent->h_addr_list[i], &addr[i].sin6_addr, addrlen);
			}
			ent->h_addr_list[i] = (char*)((BYTE*)ent->h_addr_list[i] - head);
		}
		base += (addrnum * addrlen);

		//name
		ent->h_name = (char*)(base);
		memcpy(ent->h_name, hostname.c_str(), hostname.length() + 1);
		//type
		ent->h_addrtype = AF_INET;
		//addr length  4 or 16
		ent->h_length = addrlen;

		//offset
		if (ent->h_addr_list != NULL)
			ent->h_addr_list = (char**)((BYTE*)ent->h_addr_list - head);
		if (ent->h_aliases != NULL)
			ent->h_aliases = (char**)((BYTE*)ent->h_aliases - head);
		if (ent->h_name != NULL)
			ent->h_name = (char*)((BYTE*)ent->h_name - head);
	}
	

	return blob;
}

std::wstring Util::GetModulePath(HMODULE h)
{
	wchar_t strModule[0x1000] = { 0 };

	DWORD len = ::GetModuleFileNameW(h, strModule, (sizeof(strModule) / sizeof(wchar_t)) - 1);

	std::wstring ret;

	if (len > 0)
	{
		strModule[len] = L'\0';
		ret.resize(len);
		memcpy(&ret[0], strModule, len * sizeof(wchar_t));

		std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
	}


	return ret;
}


std::wstring Util::GetCurrentExePath()
{
	return GetModulePath(GetModuleHandleW(NULL));
}
std::wstring Util::Path2Dir(const std::wstring& path)
{
	size_t len = path.length();
	while (--len > 0)
	{
		if (path[len] == L'\\' || path[len] == L'/')
			break;
	}
	std::wstring ret;

	if (len > 0)
	{
		ret.resize(len);
		memcpy(&ret[0], path.c_str(), len * sizeof(wchar_t));
	}
	return ret;
}
std::wstring Util::GetCurrentExeDir()
{
	return Path2Dir(GetCurrentExePath());
}

std::string UtilString::TrimSpace(IN const std::string& str)								//移除首尾空格
{
	//移除空格之类的
	std::string s = str;
	if (!s.empty())
	{
		s.erase(0, s.find_first_not_of(' '));
		s.erase(s.find_last_not_of(' ') + 1);
	}
	if (!s.empty())
	{
		s.erase(0, s.find_first_not_of('\r'));
		s.erase(s.find_last_not_of('\r') + 1);
	}
	if (!s.empty())
	{
		s.erase(0, s.find_first_not_of('\n'));
		s.erase(s.find_last_not_of('\n') + 1);
	}
	if (!s.empty())
	{
		s.erase(0, s.find_first_not_of('\t'));
		s.erase(s.find_last_not_of('\t') + 1);
	}
	return s;
}
std::wstring UtilString::TrimSpace(IN const std::wstring& str)							//移除首尾空格
{
	std::wstring s = str;
	if (!s.empty())
	{
		s.erase(0, s.find_first_not_of(L' '));
		s.erase(s.find_last_not_of(L' ') + 1);
	}
	if (!s.empty())
	{
		s.erase(0, s.find_first_not_of(L'\r'));
		s.erase(s.find_last_not_of(L'\r') + 1);
	}
	if (!s.empty())
	{
		s.erase(0, s.find_first_not_of(L'\n'));
		s.erase(s.find_last_not_of(L'\n') + 1);
	}
	if (!s.empty())
	{
		s.erase(0, s.find_first_not_of(L'\t'));
		s.erase(s.find_last_not_of(L'\t') + 1);
	}
	//移除空格
	return s;
}
std::string UtilString::ToLower(IN const std::string& str)					//移除首尾空格，并转换到小写
{
	std::string tmp(str.length(), '\0');
	std::transform(str.begin(), str.end(), tmp.begin(), ::tolower);
	return tmp;
}
std::wstring UtilString::ToLower(IN const std::wstring& str)					//移除首尾空格，并转换到小写
{
	std::wstring tmp(str.length(), L'\0');
	std::transform(str.begin(), str.end(), tmp.begin(), ::tolower);
	return tmp;
}
bool UtilString::StartWith(IN const std::string& str, IN const std::string& val)	//字符串判断
{
	int len_str = (int)str.length();
	int len_findit = (int)val.length();

	//要找的字符串比源字符串长
	if (len_findit > len_str)
		return false;

	//被找字符串为0时
	if (len_findit == 0)
	{
		return len_str == 0;
	}

	//被找字符串不为0时
	for (int n = 0; n < len_findit; n++)
	{
		if (str[n] != val[n])
			return false;
	}

	return true;
}
bool UtilString::StartWith(IN const std::wstring& str, IN const std::wstring& val)	//字符串判断
{
	int len_str = (int)str.length();
	int len_findit = (int)val.length();

	//要找的字符串比源字符串长
	if (len_findit > len_str)
		return false;

	//被找字符串为0时
	if (len_findit == 0)
	{
		return len_str == 0;
	}

	//被找字符串不为0时
	for (int n = 0; n < len_findit; n++)
	{
		if (str[n] != val[n])
			return false;
	}

	return true;
}
bool UtilString::EndWith(IN const std::string& str, IN const std::string& val)		//判断是否以str2结束
{
	int len_str = (int)str.length();
	int len_findit = (int)val.length();

	//要找的字符串比源字符串长
	if (len_findit > len_str)
		return false;

	//被找字符串为0时
	if (len_findit == 0)
	{
		return len_str == 0;
	}

	//被找字符串不为0时
	for (int n = len_findit - 1; n >= 0; n--)
	{
		if (str[len_str - len_findit + n] != val[n])
			return false;
	}

	return true;
}
bool UtilString::EndWith(IN const std::wstring& str, IN const std::wstring& val)	//判断是否以str2结束
{
	int len_str = (int)str.length();
	int len_findit = (int)val.length();

	//要找的字符串比源字符串长
	if (len_findit > len_str)
		return false;

	//被找字符串为0时
	if (len_findit == 0)
	{
		return len_str == 0;
	}

	//被找字符串不为0时
	for (int n = len_findit - 1; n >= 0; n--)
	{
		if (str[len_str - len_findit + n] != val[n])
			return false;
	}

	return true;
}
std::string UtilString::w2s(const std::wstring &wstr)
{
	std::string return_value;

	if (!wstr.empty())
	{
		//计算转换后长度
		int length = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
		if (length > 0)
		{
			return_value.resize(length);

			length = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.length(), &return_value[0], length, NULL, NULL);

			if (length <= 0)
				return_value.clear();
			else
				return_value.resize(length);

			return_value.resize(length);
		}
	}
	return return_value;
}

std::wstring UtilString::s2w(const std::string &str)
{
	std::wstring return_value;
	if (!str.empty())
	{
		int length = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), NULL, 0);

		return_value.resize(length);

		length = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), &return_value[0], length);	//std::string会自动保留结束符

		if (length <= 0)
			return_value.clear();
		else
			return_value.resize(length);

	}
	return return_value;
}