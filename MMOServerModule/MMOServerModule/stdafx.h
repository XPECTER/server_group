// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Winmm.lib")
#pragma comment (lib, "mysqlclient.lib")

#include "targetver.h"


#include <stdio.h>
#include <tchar.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <process.h>
#include <conio.h>

#include <map>
#include <vector>
#include <list>
#include <stack>



// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#include "../../mylibrary/APIHook.h"
#include "../../mylibrary/CrashDump.h"

#include "../../mylibrary/MemoryPool.h"
#include "../../mylibrary/MemoryPoolTLS.h"
#include "../../mylibrary/LockFreeStack.h"
#include "../../mylibrary/LockFreeStack.h"

#include "../../mylibrary/Packet.h"
#include "../../mylibrary/StreamQueue.h"

#include "../../mylibrary/Log.h"
#include "../../mylibrary/ConfigParser.h"
#include "../../mylibrary/Profiler.h"

#include "../../mylibrary/LanClient.h"
#include "../../mylibrary/DBConnector.h"

#include "defines.h"
#include "Config.h"
#include "CommonProtocol.h"


typedef __int64 CLIENT_ID;