#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
#include <ws2spi.h>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <windows.h>


//STL
#include <string>
#include <vector>
#include <map>
//#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <algorithm>

//project
#include "defines.h"
#include "util_amd64.h"
#include "Util.h"
#include "IPMapping.h"
#include "NSContext.h"

//lib
#pragma comment(lib, "ws2_32.lib")
