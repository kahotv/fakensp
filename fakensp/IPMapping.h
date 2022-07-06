#pragma once
class IPMapping
{
public:
	/* IPv4 */
	// ��Ӽ�¼
	bool v4GeneralAndAdd(IN const std::wstring& name, IN DWORD maxerror, OUT IN_ADDR& ip);
	// ��IPȡ����
	bool v4GetNameByIP(IN const IN_ADDR& ip, OUT std::wstring& name);
	// ������ȡIP
	bool v4GetIPByName(IN const std::wstring& name, OUT IN_ADDR& ip);
	// ɾ����¼
	void v4Delete(IN const IN_ADDR& ip);

	/* IPv6 */
	// ��Ӽ�¼
	bool v6GeneralAndAdd(IN const std::wstring& name, IN DWORD maxerror, OUT IN6_ADDR& ip);
	// ��IPȡ����
	bool v6GetNameByIP(IN const IN6_ADDR& ip, OUT std::wstring& name);
	// ������ȡIP
	bool v6GetIPByName(IN const std::wstring& name, OUT IN6_ADDR& ip);
	// ɾ����¼
	void v6Delete(IN const IN6_ADDR& ip);
private:

	// ������ȡIP������ȫ û������
	bool v4GetIPByNameUnsafe(IN const std::wstring& name, OUT IN_ADDR& ip);
	// ������ȡIP������ȫ û������
	bool v6GetIPByNameUnsafe(IN const std::wstring& name, OUT IN6_ADDR& ip);

	// �������IPv4��¼������ȫ û������
	bool v4TryAddUnsafe(IN const IN_ADDR& ip, IN const std::wstring& name);
	// �������IPv6��¼������ȫ û������
	bool v6TryAddUnsafe(IN const IN6_ADDR& ip, IN const std::wstring& name);
	/* Virtual IP Address */
	//��������IPv4 Loopback��ַ
	static void GetVirtualAddrIPv4(IN_ADDR *ip);
	//��������IPv6 Loopback��ַ
	static void GetVirtualAddrIPv6(IN6_ADDR *ip);
	//��������IPv4 Loopback��ַ�����ڷ�������IPv4��IPv6�ļ�������
	static DWORD GetVirtualAddrDWORD(DWORD* now);

private:
	std::shared_timed_mutex m_mapIPv4_lock;
	std::shared_timed_mutex m_mapIPv6_lock;
	std::map<std::wstring, IN_ADDR> m_mapIPv4N2A;					//Name to Addr
	std::map< std::wstring, IN6_ADDR> m_mapIPv6N2A;					//Name to Addr
	std::map<IN_ADDR, std::wstring, IN_ADDR_CMPARE> m_mapIPv4A2N;	//Addr to Name
	std::map<IN6_ADDR, std::wstring, IN6_ADDR_CMPARE> m_mapIPv6A2N;	//Addr to Name
};
