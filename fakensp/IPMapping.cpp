#include "stdafx.h"
#include "IPMapping.h"

bool IPMapping::v4GeneralAndAdd(IN const std::wstring& name, IN DWORD maxerror, OUT IN_ADDR& ip)
{
	DWORD error_now = 0;
	IN_ADDR ip_tmp = {};

	do
	{
		std::unique_lock<std::shared_timed_mutex>lock(m_mapIPv4_lock);

		if (v4GetIPByNameUnsafe(name, ip_tmp))
		{
			ip = ip_tmp;
			break;
		}

		GetVirtualAddrIPv4(&ip_tmp);
		if (!v4TryAddUnsafe(ip_tmp, name))
		{
			error_now++;
		}
		else
		{
			error_now = 0;
			ip = ip_tmp;
			break;
		}

	} while (error_now < maxerror);

	return error_now == 0;
}
bool IPMapping::v4GetNameByIP(IN const IN_ADDR& ip, OUT std::wstring& name)
{
	std::shared_lock<std::shared_timed_mutex> lock(m_mapIPv4_lock);
	auto it = m_mapIPv4A2N.find(ip);
	if (it != m_mapIPv4A2N.end())
	{
		name = it->second;
		return true;
	}
	return false;
}
bool IPMapping::v4GetIPByName(IN const std::wstring& name, OUT IN_ADDR& ip)
{
	std::shared_lock<std::shared_timed_mutex> lock(m_mapIPv4_lock);
	return v4GetIPByNameUnsafe(IN name, OUT ip);
}
void IPMapping::v4Delete(IN const IN_ADDR& ip)
{
	std::unique_lock<std::shared_timed_mutex> lock(m_mapIPv4_lock);
	auto it = m_mapIPv4A2N.find(ip);
	if (it != m_mapIPv4A2N.end())
	{
		auto it2 = m_mapIPv4N2A.find(it->second);
		if (it2 != m_mapIPv4N2A.end())
		{
			m_mapIPv4N2A.erase(it2);
		}
		m_mapIPv4A2N.erase(it);
	}
}

bool IPMapping::v6GeneralAndAdd(IN const std::wstring& name, IN DWORD maxerror, OUT IN6_ADDR& ip)
{
	DWORD error_now = 0;
	IN6_ADDR ip_tmp = {};

	do
	{
		std::shared_lock<std::shared_timed_mutex> lock(m_mapIPv6_lock);

		if (v6GetIPByNameUnsafe(name, ip_tmp))
		{
			ip = ip_tmp;
			break;
		}

		GetVirtualAddrIPv6(&ip_tmp);
		if (!v6TryAddUnsafe(ip_tmp, name))
		{
			error_now++;
		}
		else
		{
			error_now = 0;
			ip = ip_tmp;
			break;
		}

	} while (error_now < maxerror);

	return error_now == 0;
}
bool IPMapping::v6GetNameByIP(IN const IN6_ADDR& ip, OUT std::wstring& name)
{
	std::shared_lock<std::shared_timed_mutex> lock(m_mapIPv6_lock);
	auto it = m_mapIPv6A2N.find(ip);
	if (it != m_mapIPv6A2N.end())
	{
		name = it->second;
		return true;
	}
	return false;
}
bool IPMapping::v6GetIPByName(IN const std::wstring& name, OUT IN6_ADDR& ip)
{
	std::shared_lock<std::shared_timed_mutex> lock(m_mapIPv6_lock);
	return v6GetIPByNameUnsafe(IN name, OUT ip);
}

void IPMapping::v6Delete(IN const IN6_ADDR& ip)
{
	std::unique_lock<std::shared_timed_mutex> lock(m_mapIPv6_lock);
	auto it = m_mapIPv6A2N.find(ip);
	if (it != m_mapIPv6A2N.end())
	{
		auto it2 = m_mapIPv6N2A.find(it->second);
		if (it2 != m_mapIPv6N2A.end())
		{
			m_mapIPv6N2A.erase(it2);
		}
		m_mapIPv6A2N.erase(it);
	}
}

bool IPMapping::v4GetIPByNameUnsafe(IN const std::wstring& name, OUT IN_ADDR& ip)
{
	auto it = m_mapIPv4N2A.find(name);
	if (it != m_mapIPv4N2A.end())
	{
		ip = it->second;
		return true;
	}
	return false;
}
bool IPMapping::v6GetIPByNameUnsafe(IN const std::wstring& name, OUT IN6_ADDR& ip)
{
	auto it = m_mapIPv6N2A.find(name);
	if (it != m_mapIPv6N2A.end())
	{
		ip = it->second;
		return true;
	}
	return false;
}

bool IPMapping::v4TryAddUnsafe(IN const IN_ADDR& ip, IN const std::wstring& name)
{
	auto it = m_mapIPv4A2N.find(ip);
	auto it2 = m_mapIPv4N2A.find(name);
	if (it == m_mapIPv4A2N.end() && it2 == m_mapIPv4N2A.end())
	{
		m_mapIPv4A2N.emplace(ip, name);
		m_mapIPv4N2A.emplace(name, ip);
		return true;
	}
	return false;
}
bool IPMapping::v6TryAddUnsafe(IN const IN6_ADDR& ip, IN const std::wstring& name)
{
	auto it = m_mapIPv6A2N.find(ip);
	auto it2 = m_mapIPv6N2A.find(name);
	if (it == m_mapIPv6A2N.end() && it2 == m_mapIPv6N2A.end())
	{
		m_mapIPv6A2N.emplace(ip, name);
		m_mapIPv6N2A.emplace(name, ip);
		return true;
	}
	return false;
}

DWORD IPMapping::GetVirtualAddrDWORD(DWORD* now)
{
	//这个函数不管是否已经被使用，只负责循环生成合法的IP
	const DWORD ipBegin = 0x7F090000;
	const DWORD ipEnd = 0x7FFE0000;
	DWORD ret = 0;

	do
	{
		InterlockedCompareExchange(now, ipBegin, 0);

		do
		{
			//跳过X.X.X.X格式里，X为0的项
			ret = InterlockedIncrement(now);
		} while ((ret & 0x000000FF) == 0);

		//循环
		ret = InterlockedCompareExchange(now, ipBegin, ipEnd);
		if (ret == ipEnd)
			continue;

	} while (false);

	return ret;
}
void IPMapping::GetVirtualAddrIPv4(IN_ADDR* ip)
{
	static DWORD ipNow = 0;

	DWORD ret = GetVirtualAddrDWORD(&ipNow);

	((DWORD*)ip)[0] = htonl(ret);
}
void IPMapping::GetVirtualAddrIPv6(IN6_ADDR *ip)
{
	//这个函数不管是否已经被使用，只负责循环生成合法的IP

	static DWORD ipNow = 0;
	DWORD ret = GetVirtualAddrDWORD(&ipNow);

	((DWORD*)ip)[0] = 0;
	((DWORD*)ip)[1] = 0;
	((DWORD*)ip)[2] = 0;
	((DWORD*)ip)[3] = htonl(ret);

}