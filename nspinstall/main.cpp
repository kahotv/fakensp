// mynspinstall.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

std::wstring GetCurrentExtDir()
{
    wchar_t strModule[0x1000];

    DWORD len = ::GetModuleFileNameW(NULL, strModule, sizeof(strModule) / sizeof(wchar_t));
    if (len > 0)
    {
        while (--len > 0)
        {
            if (strModule[len] == L'\\' || strModule[len] == L'/')
                break;
        }
    }

    std::wstring ret;

    if (len > 0)
    {
        ret = std::wstring(strModule, len + 1);
    }

    return ret;
}

void show()
{

    std::vector< WSANAMESPACE_INFO2W> list;
    if (!(Util::NspGetAll(list)))
    {
        printf("ȡNSP�б�ʧ�� err: %d", WSAGetLastError());
        return;
    }
    setlocale(LC_ALL, "chs");
    wprintf(L"NSP����: %zd\n", list.size());
    for (size_t i = 0; i < list.size(); i++) 
    {
        if (list[i].dwNameSpace != NS_DNS && list[i].dwNameSpace != NS_ALL)
        {
            continue;
        }
        WCHAR GuidString[40] = { 0 };
        auto iRet = StringFromGUID2(list[i].NSProviderId, (LPOLESTR)&GuidString, 39);
        if (iRet == 0)
            wprintf(L"StringFromGUID2 failed\n");
        else
            wprintf(L"NameSpace[%zu] GUID = %ws\n", i, GuidString);

        wprintf(L"NameSpace[%zu] = ", i);
        switch (list[i].dwNameSpace) {
        case NS_DNS:
            wprintf(L"Domain Name System (NS_DNS)\n");
            break;
        case NS_WINS:
            wprintf(L"Windows Internet Naming Service (NS_WINS)\n");
            break;
        case NS_NETBT:
            wprintf(L"NetBIOS (NS_NETBT)\n");
            break;
        case NS_NTDS:
            wprintf(L"Windows NT Directory Services (NS_NTDS)\n");
            break;
        case NS_NLA:
            wprintf(L"Network Location Awareness (NS_NLA)\n");
            break;
            // following values only defined on Vista and later
#if(_WIN32_WINNT >= 0x0600)
        case NS_BTH:
            wprintf(L"Bluetooth (NS_BTH)\n");
            break;
        case NS_EMAIL:
            wprintf(L"Email (NS_EMAIL)\n");
            break;
        case NS_PNRPNAME:
            wprintf(L"Peer-to-peer (NS_PNRPNAME)\n");
            break;
        case NS_PNRPCLOUD:
            wprintf(L"Peer-to-peer collection (NS_PNRPCLOUD)\n");
            break;
#endif
        default:
            wprintf(L"Other value (%u)\n", list[i].dwNameSpace);
            break;
        }

        if (list[i].fActive)
            wprintf(L"Namespace[%zu] is �״̬\n", i);
        else
            wprintf(L"Namespace[%zu] is �ǻ״̬\n", i);

        wprintf(L"NameSpace[%zu] Version = %u\n", i, list[i].dwVersion);

        wprintf(L"Namespace[%zu] Identifier = %ws\n\n", i, list[i].lpszIdentifier.c_str());
    }
}

void uninstall()
{
    INT ret = WSCUnInstallNameSpace(&MY_NAMESPACE_GUID);
    if (ret == SOCKET_ERROR)
    {
        printf("ж��FakeNSPʧ�� err: %d\n", WSAGetLastError());
    }
    else
    {
        printf("ж��FakeNSP�ɹ�\n");
    }
}

void install(bool totop, wchar_t* dllname)
{
    //show();
    uninstall();
    //show();
    std::wstring path = GetCurrentExtDir() + dllname;
    INT ret = WSCInstallNameSpace(L"Fake NSP",
        &path[0], NS_DNS, 1, &MY_NAMESPACE_GUID);
    if (ret == SOCKET_ERROR)
    {
        printf("��װʧ�� err: %d\n",
            WSAGetLastError());
    }
    else
    {
        printf("��װFakeNSP�ɹ�\n");
        if (totop)
        {
            //�������ȼ�
            do
            {
                std::vector<GUID> listGuid;
                std::vector<GUID> listGuidKeep = { MY_NAMESPACE_GUID,Util::ParseGuid(L"{22059D40-7E9E-11CF-AE5A-00AA00A7112B}") };
                std::vector<WSANAMESPACE_INFO2W> listNsp, listNsp2;

                //��ȡNSP�б�
                if (!Util::NspGetAll(listNsp))
                    break;
                //ֻ�����Լ��ĺ�ϵͳ��NSP
                //Util::NspKeepList(listNsp, listGuidKeep, NS_DNS, listNsp2);
                //���Լ���NSP�������ȼ�
				Util::NspOrder(listNsp, MY_NAMESPACE_GUID, 0, listNsp2);
                //����˳��
                Util::NspToGuidList(listNsp2, listGuid);
                INT r = WSCWriteNameSpaceOrder(&listGuid[0], listGuid.size());
                if (r == 0)
                {
                    printf("NSP����ɹ�\n");
                }
                else
                {
                    printf("NSP����ʧ��  ret: %d, err: %d\n", r, WSAGetLastError());
                }

            } while (false);
        }

    }
}

int main(int argc, char** argv)
{
    WSADATA        wsd;

    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        printf("WSAStartup() ʧ�� err: %d\n", GetLastError());
        return -1;
    }

    while (true)
    {
        std::string cmd;
        
        printf("����ָ��: \n1 ��װ\n2 ж��\n3 չʾ\n");
        std::cin >> cmd;
        if (cmd == "1")
        {
#ifdef _WIN64
            install(true, L"fakensp64.dll");  // Install the name space provider
#else
            install(true, L"fakensp.dll");  // Install the name space provider
#endif
        }
        else if (cmd == "2")
        {
            uninstall();
        }
        else if(cmd == "3")
        {
            show();
        }
        else 
        {
            printf("δ֪������\n");
        }
    }

    WSACleanup();
    //getchar();
    return 0;
}

