// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
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
// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
