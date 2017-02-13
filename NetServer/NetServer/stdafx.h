// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma comment (lib, "Ws2_32.lib")
#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <WinSock2.h>
#include <Windows.h>
#include <signal.h>
#include <crtdbg.h>
#include <Psapi.h>
#include <DbgHelp.h>
#include <TlHelp32.h>
#include <WS2tcpip.h>
#include <process.h>
#include <time.h>
#include <mstcpip.h>
#include <conio.h>
#include <locale.h>
#include <map>
#include <list>

#include "CommonProtocol.h"

#include "../../mylibrary/APIHook.h"
#include "../../mylibrary/CrashDump.h"
#include "../../mylibrary/MemoryPool.h"
#include "../../mylibrary/MemoryPoolTLS.h"
#include "../../mylibrary/LockFreeStack.h"
#include "../../mylibrary/LockFreeQueue.h"
#include "../../mylibrary/Packet.h"
#include "../../mylibrary/StreamQueue.h"
#include "../../mylibrary/Profiler.h"
#include "../../mylibrary/Log.h"
#include "../../mylibrary/ConfigParser.h"
#include "../../mylibrary/NetServer.h"
#include "../../mylibrary/LanClient.h"
// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
