#include "stdafx.h"
#include "Util.h"

bool Util::GuidContant(const std::vector<GUID>& inListGuid, const GUID& inGuid)
{
	for (const auto& item : inListGuid)
	{
		if (item == inGuid)
			return true;
	}

	return false;
}
bool Util::GuidSwap(std::vector<GUID>& list, const GUID& a, const GUID& b)
{
	//先查找两个GUID顺序
	int pos_a = -1, pos_b = -1;
	for (int i = 0; i < (int)list.size(); i++)
	{
		if (list[i] == a)
		{
			pos_a = i;
		}
		else if (list[i] == b) {
			pos_b = i;
		}

		if (pos_a != -1 && pos_b != -1)
		{
			break;
		}
	}

	if (pos_a != -1 && pos_b != -1)
	{
		//交换
		std::swap(list[pos_a], list[pos_b]);
		return true;
	}
	return false;
}
GUID Util::ParseGuid(const std::wstring& strGuid)
{
	GUID guid = {};
	IIDFromString(strGuid.c_str(), &guid);
	return guid;
}

bool Util::NspGetAll(std::vector<WSANAMESPACE_INFO2W>& list)
{
	DWORD buflen = 0;

	WSAEnumNameSpaceProvidersW(&buflen, NULL);
	if (buflen > 0)
	{
		WSANAMESPACE_INFOW* lpProviderInfo = (WSANAMESPACE_INFOW*)alloca(buflen);
		INT num = WSAEnumNameSpaceProvidersW(&buflen, lpProviderInfo);
		if (num < 0)
			return false;

		list.resize(num);
		for (int i = 0; i < num; i++)
		{
			list[i].NSProviderId = lpProviderInfo[i].NSProviderId;
			list[i].dwNameSpace = lpProviderInfo[i].dwNameSpace;
			list[i].fActive = lpProviderInfo[i].fActive;
			list[i].lpszIdentifier = lpProviderInfo[i].lpszIdentifier;
		}

		return true;
	}

	return false;
}
bool Util::NspContant(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, const GUID& inGuid)
{
	for (const auto& item : inListNsp)
	{
		if (item.NSProviderId == inGuid)
			return true;
	}

	return false;
}
void Util::NspOrder(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, const GUID& inGuid, int index, std::vector<WSANAMESPACE_INFO2W>& outListNsp)
{
	outListNsp.clear();
	outListNsp.resize(inListNsp.size());

	//把指定的guid放到第index位（index以0开始）
	for (size_t i = 0, n = 0; i < inListNsp.size(); i++)
	{
		if (inListNsp[i].NSProviderId == inGuid) 
		{
			outListNsp[index] = inListNsp[i];
		}
		else 
		{
			if (n == index)
			{
				n++;	//跳过index
			}
			outListNsp[n++] = inListNsp[i];
		}
	}
}
void Util::NspRemoveList(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, const std::vector<GUID>& inListGuidDelete, std::vector<WSANAMESPACE_INFO2W>& outListNsp)
{
	outListNsp.clear();
	outListNsp.reserve(inListNsp.size());

	//把inListNspDelte之外的放入outListNsp
	for (const auto& item : inListNsp)
	{
		bool find = false;
		for (const auto& del : inListGuidDelete)
		{
			if (del == item.NSProviderId)
				find = true;
		}

		if (!find)
			outListNsp.push_back(item);
	}
}
void Util::NspKeepList(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, const std::vector<GUID>& inListGuidKeep, DWORD inNspType, std::vector<WSANAMESPACE_INFO2W>& outListNsp)
{
	outListNsp.clear();
	outListNsp.reserve(inListNsp.size());
	for (const auto& item : inListNsp)
	{
		bool contant = GuidContant(inListGuidKeep, item.NSProviderId);

		if (!contant && item.dwNameSpace != inNspType)
		{
			outListNsp.push_back(item);		//不是指定的guid，且不是指定的type
		}
		else if (contant && item.dwNameSpace == inNspType)
		{
			outListNsp.push_back(item);		//是指定的guid，且是指定的type
		}
	}
}
void Util::NspToGuidList(const std::vector<WSANAMESPACE_INFO2W>& inListNsp, std::vector<GUID>& outListGuid)
{
	outListGuid.clear();
	outListGuid.reserve(inListNsp.size());
	for (const auto& item : inListNsp)
	{
		outListGuid.push_back(item.NSProviderId);
	}
}
