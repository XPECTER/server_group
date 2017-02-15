// NetServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//
#pragma comment (lib, "Ws2_32.lib")

#include "stdafx.h"
#include "LanClient_Chat.h"
#include "ChatServer.h"
#include "main.h"

CMemoryPoolTLS<CPacket> CPacket::PacketPool(300, true);
BYTE CPacket::_packetCode = 0;
BYTE CPacket::_packetKey_1 = 0;
BYTE CPacket::_packetKey_2 = 0;

long CCrashDump::_DumpCount = 0;
CCrashDump crashDump;
bool g_bPrintConfig = false;

CConfigData g_ConfigData;


int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "");
	LoadConfigData();

	CChatServer chatserver;
	SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"//--------- Start ChatServer ---------//");
	

	Profiler_Init();

	DWORD keyResult = 0;

	tm t;
	time_t startTime = time(NULL);
	localtime_s(&t, &startTime);

	while (true)
	{
		chatserver.Start();
		keyResult = KeyProcess();

		/*if (1 == keyResult)
			chatserver.Stop();
		else if (2 == keyResult)
			chatserver.Start();*/

		Sleep(998);
		system("cls");
		wprintf_s(L"SERVER ON TIME : [%04d-%02d-%02d %02d:%02d:%02d]", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
		wprintf_s(L"\n===============================================================\n");
		
		wprintf_s(L"Session Count : %d\n", chatserver.GetClientCount());
		wprintf_s(L"Packet Pool Alloc Size : %d\n", CPacket::PacketPool.GetAllocCount());
		wprintf_s(L"Using Packet : %d\n\n", CPacket::PacketPool.GetUseCount());
		
		wprintf_s(L"Update Message Pool Alooc Size : %d\n", chatserver.GetUpdateMessagePoolAllocSize());
		wprintf_s(L"Update Message Queue Remain Size : %d\n\n", chatserver.GetUpdateMessageQueueSize());

		wprintf_s(L"Player Pool Alloc Size : %d\n", chatserver._MemoryPool_Player.GetAllocCount());
		wprintf_s(L"Player Count : %d\n\n", chatserver.GetPlayerCount());

		wprintf_s(L"Accept Total : %I64u\n", chatserver._iAcceptTotal);
		wprintf_s(L"Accept TPS : %d\n", chatserver._iAcceptTPS);
		wprintf_s(L"Update TPS : %d\n\n", chatserver._Monitor_UpdateTPS);

		wprintf_s(L"RecvPacket TPS : %d\n", chatserver._iRecvPacketTPS);
		wprintf_s(L"SendPacket TPS : %d\n\n", chatserver._iSendPacketTPS);

		wprintf_s(L"RecvPacket TPS from LoginServer : %d\n", chatserver.GetRecvCountFromLogin());

		PrintConfig();
	}

	return 0;
}


DWORD KeyProcess(void)
{
	int key;

	if (0 != _kbhit())
	{
		key = _getch();

		if ('Q' == key || 'q' == key)
		{
			return 1;
		}
		else if ('P' == key || 'p' == key)
		{
			Profiler_Print();
		}
		else if ('C' == key || 'c' == key)
		{
			Profiler_Clear();
		}
		else if ('S' == key || 's' == key)
		{
			return 2;
		}
		else if ('I' == key || 'i' == key)
		{
			if (true == g_bPrintConfig)
				g_bPrintConfig = false;
			else
				g_bPrintConfig = true;
		}
	}
	return 0;
}

bool LoadConfigData(void)
{
	CConfigParser parser;
	wchar_t szBlock[32] = { 0, };
	wchar_t szKey[32] = { 0, };

	if (!parser.OpenConfigFile(L"ChatServerConfig.ini"))
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Config file open failed");
		return false;
	}
	else
	{
		wsprintf(szBlock, L"NETWORK");
		if (!parser.MoveConfigBlock(szBlock))
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s BLOCK in Config file", szBlock);
			return false;
		}
		else
		{
			wsprintf(szKey, L"SERVER_GROUP_NAME");
			if (!parser.GetValue(szKey, g_ConfigData._szServerGroupName, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"THREAD_NUM");
			if (!parser.GetValue(szKey, &g_ConfigData._iThreadNum))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"CHAT_SERVER_BIND_IP");
			if (!parser.GetValue(szKey, g_ConfigData._szChatServerBindIP, 16))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"CHAT_SERVER_BIND_PORT");
			if (!parser.GetValue(szKey, &g_ConfigData._iChatServerBindPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"CHAT_SERVER_NAGLE_OPT");
			if (!parser.GetValue(szKey, &g_ConfigData._bChatServerNagleOpt))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_IP");
			if (!parser.GetValue(szKey, g_ConfigData._szLoginServerIP, 16))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_PORT");
			if (!parser.GetValue(szKey, &g_ConfigData._iLoginServerPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_NAGLE_OPT");
			if (!parser.GetValue(szKey, &g_ConfigData._bLoginServerNagleOpt))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"MONITORING_SERVER_IP");
			if (!parser.GetValue(szKey, g_ConfigData._szMonitoringServerIP, 16))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"MONITORING_SERVER_PORT");
			if (!parser.GetValue(szKey, &g_ConfigData._iMonitoringServerPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"MONITORING_SERVER_NAGLE_OPT");
			if (!parser.GetValue(szKey, &g_ConfigData._bMonitoringServerNagleOpt))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}
		}

		wsprintf(szBlock, L"SYSTEM");
		if (!parser.MoveConfigBlock(szBlock))
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s BLOCK in Config file", szBlock);
			return false;
		}
		else
		{
			wsprintf(szKey, L"CLIENT_MAX");
			if (!parser.GetValue(szKey, &g_ConfigData._iClientMax))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"PACKET_CODE");
			if (!parser.GetValue(szKey, &g_ConfigData._iPacketCode))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"PACKET_KEY1");
			if (!parser.GetValue(szKey, &g_ConfigData._iPacketKey1))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"PACKET_KEY2");
			if (!parser.GetValue(szKey, &g_ConfigData._iPacketKey2))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			CPacket::SetEncryptCode((BYTE)g_ConfigData._iPacketCode, (BYTE)g_ConfigData._iPacketKey1, (BYTE)g_ConfigData._iPacketKey2);
		}

		wsprintf(szBlock, L"LOG");
		if (!parser.MoveConfigBlock(szBlock))
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s BLOCK in Config file", szBlock);
			return false;
		}
		else
		{
			wsprintf(szKey, L"LOG_LEVEL");
			if (!parser.GetValue(szKey, &g_ConfigData._iLogLevel))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"PRINT_CONSOLE");
			if (!parser.GetValue(szKey, &g_ConfigData._bPrintConsole))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"PRINT_FILE");
			if (!parser.GetValue(szKey, &g_ConfigData._bPrintFile))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"PRINT_DATABASE");
			if (!parser.GetValue(szKey, &g_ConfigData._bPrinfDatabase))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			LOG::iLogLevel = g_ConfigData._iLogLevel;
			LOG::bConsole = g_ConfigData._bPrintConsole;
			LOG::bFile = g_ConfigData._bPrintFile;
			LOG::bDatabase = g_ConfigData._bPrinfDatabase;
		}
	}

	return true;
}

void PrintConfig(void)
{
	CConfigData *pConfig = nullptr;

	if (!g_bPrintConfig)
	{
		wprintf_s(L"Press [i] : See Config\n\n");
	}
	else
	{
		pConfig = &g_ConfigData;

		wprintf_s(L"Press [i] : Hide Config\n\n");
		wprintf_s(L"CHAT_BIND_IP\t\t: %s\n", pConfig->_szChatServerBindIP);
		wprintf_s(L"CHAT_BIND_PORT\t: %d\n", pConfig->_iChatServerBindPort);
		wprintf_s(L"CHAT_NAGLE_OPT\t: %s\n", pConfig->_bChatServerNagleOpt == true ? L"ON" : L"OFF");
		wprintf_s(L"CLIENT_MAX\t: %d\n", pConfig->_iClientMax);
		wprintf_s(L"PACKET_CODE\t: 0x%x\n", pConfig->_iPacketCode);
		wprintf_s(L"PACKET_KEY1\t: 0x%x\n", pConfig->_iPacketKey1);
		wprintf_s(L"PACKET_KEY2\t: 0x%x\n", pConfig->_iPacketKey2);
	}

	return;
}