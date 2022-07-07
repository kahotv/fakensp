#pragma once
class Util
{
public:
	/* GUID */
	// ����inGuid�Ƿ���inListGuid�б���
	static bool Util::GuidContant(const std::vector<GUID>& inListGuid, const GUID& inGuid);
	// ����list��a��b��˳�򣬷����Ƿ�ɹ�
	static bool Util::GuidSwap(std::vector<GUID>& list, const GUID& a, const GUID& b);
	static GUID Util::ParseGuid(const std::wstring& strGuid);

	/* NSP */
	// ��ȡ���е�NSP
	static bool Util::NspGetAll(std::vector<WSANAMESPACE_INFO2W>& list);
	// ��inListNsp������inGuid�������Ƿ����
	static bool Util::NspContant(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, const GUID& inGuid);
	// ��ָ��GUID��NSP���õ�listGuid���һλ
	static void Util::NspOrder(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, const GUID& inGuid, int index, std::vector<WSANAMESPACE_INFO2W>& outListNsp);
	// �Ƴ�ָ����guid
	static void Util::NspRemoveList(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, const std::vector<GUID>& inListGuidDelete, std::vector<WSANAMESPACE_INFO2W>& outListNsp);
	// ����ָ��nspType��guid��������Ƴ�
	static void Util::NspKeepList(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, const std::vector<GUID>& inListGuidKeep, DWORD inNspType, std::vector<WSANAMESPACE_INFO2W>& outListNsp);
	// ȡ��NSP�б������е�GUID������˳��
	static void Util::NspToGuidList(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, std::vector<GUID>& outListGuid);
};

