#pragma once
class Util
{
public:
	/* GUID */
	// 返回inGuid是否在inListGuid列表里
	static bool Util::GuidContant(const std::vector<GUID>& inListGuid, const GUID& inGuid);
	// 交换list里a和b的顺序，返回是否成功
	static bool Util::GuidSwap(std::vector<GUID>& list, const GUID& a, const GUID& b);
	static GUID Util::ParseGuid(const std::wstring& strGuid);

	/* NSP */
	// 获取所有的NSP
	static bool Util::NspGetAll(std::vector<WSANAMESPACE_INFO2W>& list);
	// 在inListNsp里搜索inGuid，返回是否存在
	static bool Util::NspContant(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, const GUID& inGuid);
	// 把指定GUID的NSP放置到listGuid里第一位
	static void Util::NspOrder(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, const GUID& inGuid, int index, std::vector<WSANAMESPACE_INFO2W>& outListNsp);
	// 移除指定的guid
	static void Util::NspRemoveList(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, const std::vector<GUID>& inListGuidDelete, std::vector<WSANAMESPACE_INFO2W>& outListNsp);
	// 保留指定nspType的guid，其余的移除
	static void Util::NspKeepList(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, const std::vector<GUID>& inListGuidKeep, DWORD inNspType, std::vector<WSANAMESPACE_INFO2W>& outListNsp);
	// 取出NSP列表里所有的GUID（保留顺序）
	static void Util::NspToGuidList(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, std::vector<GUID>& outListGuid);
};

