#pragma once
class IPMapping
{
public:
	/* IPv4 */
	// 添加记录
	bool v4GeneralAndAdd(IN const std::wstring& name, IN DWORD maxerror, OUT IN_ADDR& ip);
	// 用IP取域名
	bool v4GetNameByIP(IN const IN_ADDR& ip, OUT std::wstring& name);
	// 用域名取IP
	bool v4GetIPByName(IN const std::wstring& name, OUT IN_ADDR& ip);
	// 删除记录
	void v4Delete(IN const IN_ADDR& ip);

	/* IPv6 */
	// 添加记录
	bool v6GeneralAndAdd(IN const std::wstring& name, IN DWORD maxerror, OUT IN6_ADDR& ip);
	// 用IP取域名
	bool v6GetNameByIP(IN const IN6_ADDR& ip, OUT std::wstring& name);
	// 用域名取IP
	bool v6GetIPByName(IN const std::wstring& name, OUT IN6_ADDR& ip);
	// 删除记录
	void v6Delete(IN const IN6_ADDR& ip);
private:

	// 用域名取IP（不安全 没加锁）
	bool v4GetIPByNameUnsafe(IN const std::wstring& name, OUT IN_ADDR& ip);
	// 用域名取IP（不安全 没加锁）
	bool v6GetIPByNameUnsafe(IN const std::wstring& name, OUT IN6_ADDR& ip);

	// 尝试添加IPv4记录（不安全 没加锁）
	bool v4TryAddUnsafe(IN const IN_ADDR& ip, IN const std::wstring& name);
	// 尝试添加IPv6记录（不安全 没加锁）
	bool v6TryAddUnsafe(IN const IN6_ADDR& ip, IN const std::wstring& name);
	/* Virtual IP Address */
	//生成虚拟IPv4 Loopback地址
	static void GetVirtualAddrIPv4(IN_ADDR *ip);
	//生成虚拟IPv6 Loopback地址
	static void GetVirtualAddrIPv6(IN6_ADDR *ip);
	//生成虚拟IPv4 Loopback地址（用于分离生成IPv4和IPv6的计数器）
	static DWORD GetVirtualAddrDWORD(DWORD* now);

private:
	std::shared_timed_mutex m_mapIPv4_lock;
	std::shared_timed_mutex m_mapIPv6_lock;
	std::map<std::wstring, IN_ADDR> m_mapIPv4N2A;					//Name to Addr
	std::map< std::wstring, IN6_ADDR> m_mapIPv6N2A;					//Name to Addr
	std::map<IN_ADDR, std::wstring, IN_ADDR_CMPARE> m_mapIPv4A2N;	//Addr to Name
	std::map<IN6_ADDR, std::wstring, IN6_ADDR_CMPARE> m_mapIPv6A2N;	//Addr to Name
};
