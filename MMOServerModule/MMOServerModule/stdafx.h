// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
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

#include <my_global.h>
#include <mysql.h>

// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
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

#include "defines.h"
#include "Config.h"
#include "CommonProtocol.h"
#include "DBTypeEnum.h"