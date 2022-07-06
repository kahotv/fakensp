#pragma once

__declspec(selectany) GUID MY_NAMESPACE_GUID = { 0x55a2bd9e,0xbb30,0x11d2,{0x91,0x66,0x00,0xa0,0xc9,0xa7,0x86,0xe8} };

struct WSANAMESPACE_INFO2W
{
    GUID                NSProviderId;
    DWORD               dwNameSpace;
    BOOL                fActive;
    DWORD               dwVersion;
    std::wstring        lpszIdentifier;
};
