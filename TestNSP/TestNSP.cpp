#include "stdafx.h"

#pragma comment(lib,"ws2_32.lib")

const char* GetFamilyString(int family)
{
    if (family == AF_INET)
        return "IPv4";
    else if (family == AF_INET6)
        return "IPv6";
    else
        return "N/A";
}

std::string IPGetString(const sockaddr* sa)
{
    if (sa->sa_family != AF_INET && sa->sa_family != AF_INET6)
        return "Unknown";

    char buf[0x100] = { 0 };
    DWORD buflen = sizeof(buf);
    int addrlen = sa->sa_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);

    if (SOCKET_ERROR == WSAAddressToStringA((sockaddr*)sa, addrlen, NULL, buf, &buflen))
    {
        return "Unknown";
    }

    return buf;
}

void testGetHostByName(const char* name)
{
    hostent* hptr = gethostbyname(name);
    if (hptr == NULL) {
        printf("gethostbyname error for host: %s: %d\n", name, h_errno);
        return;
    }
    //输出主机的规范名
    printf("\tofficial: %s\n", hptr->h_name);

    //输出主机的别名
    char** pptr;
    for (pptr = hptr->h_aliases; *pptr != NULL; pptr++) {
        printf("\ttalias: %s\n", *pptr);
    }

    //输出ip地址
    switch (hptr->h_addrtype) {
    case AF_INET:
        //IP地址
        for (int i = 0; hptr->h_addr_list[i]; i++) {
            sockaddr_in si = {};
            si.sin_family = AF_INET;
            si.sin_addr = *(struct in_addr*)hptr->h_addr_list[i];
            printf("\taddress %d: %s\n", i + 1, IPGetString((sockaddr*)&si).c_str());
        }

        break;
        /*
    case AF_INET6:
        for (int i = 0; hptr->h_addr_list[i]; i++) {
            sockaddr_in6 si = {};
            si.sin6_family = AF_INET6;
            si.sin6_addr = *(struct in_addr6*)hptr->h_addr_list[i];
            printf("\taddress %d: %s\n", i + 1, IPGetString(AF_INET6, &si).c_str());
        }
        */
    default:
        printf("unknown address type\n");
        break;
    }
}

void printAddrInfoA(ADDRINFOA* info)
{
    if (info != NULL) {
        ADDRINFOA* ptr = NULL;
        int i = 0;
        for (ptr = info, i = 0; ptr != NULL; ptr = ptr->ai_next)
        {

            printf("getaddrinfoA response %d\n", i++);
            printf("\tFlags: 0x%x\n", ptr->ai_flags);
            printf("\tFamily: %s", GetFamilyString(ptr->ai_family));
            printf("\tLength of this sockaddr: %zd\n", ptr->ai_addrlen);
            printf("\tCanonical name: %s\n", ptr->ai_canonname);
            printf("\tAddress: %s\n", IPGetString(ptr->ai_addr).c_str());
        }
    }
}
void printAddrInfoW(ADDRINFOW* info)
{
    if (info != NULL) {
        ADDRINFOW* ptr = NULL;
        int i = 0;
        for (ptr = info, i = 0; ptr != NULL; ptr = ptr->ai_next)
        {
            printf("getaddrinfoW response %d\n", i++);
            printf("\tFlags: 0x%x\n", ptr->ai_flags);
            printf("\tFamily: %s", GetFamilyString(ptr->ai_family));
            printf("\tLength of this sockaddr: %zd\n", ptr->ai_addrlen);
            wprintf(L"\tCanonical name: %ws\n", ptr->ai_canonname);
            printf("\tAddress: %s\n", IPGetString(ptr->ai_addr).c_str());
        }
    }
}

void testGetAddrInfoA(int family, const char* name)
{
    addrinfo req = {};
    req.ai_family = family;
    //req.ai_socktype = 0;
    //req.ai_protocol = 0;
    req.ai_flags = AI_ALL;
    addrinfo* resp = NULL;
    INT ret = getaddrinfo(name, NULL, &req, &resp);
    printf("getaddrinfo ret :%d\n", ret);
    if (resp != NULL) 
    {
        printAddrInfoA(resp);
        freeaddrinfo(resp);
    }
}
void testGetAddrInfoW(int family, const wchar_t* name)
{
    ADDRINFOW req = {};
    req.ai_family = family;
    //req.ai_socktype = 0;
    //req.ai_protocol = 0;
    req.ai_flags = AI_ALL;
    ADDRINFOW* resp = NULL;
    INT ret = GetAddrInfoW(name, NULL, &req, &resp);
    printf("getaddrinfo ret :%d\n", ret);
    if (resp != NULL) {

        printAddrInfoW(resp);
        FreeAddrInfoW(resp);
    }
}

int main()
{
    WSADATA wsaData;
    WSAStartup(0x0101, &wsaData);
    const char* name = "www.baidu.com";
    const wchar_t* name2 = L"www.baidu.com";
    while (true)
    {
        system("pause");
        testGetHostByName(name);
        testGetAddrInfoA(AF_INET,name);
        testGetAddrInfoW(AF_INET, name2);
        printf("=============================================\n");
        testGetAddrInfoA(AF_INET6, name);
        testGetAddrInfoW(AF_INET6, name2);
    }
    WSACleanup();
    system("pause");
    return 0;
}

