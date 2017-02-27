// MMOServerModule.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "MMOSession.h"
#include "MMOServer.h"
#include "LanClient_Game.h"
#include "GameServer.h"

CMemoryPoolTLS<CPacket> CPacket::PacketPool(300, true);
CConfig g_Config;
CGameServer *g_pGameServer = nullptr;


bool LoadConfig(void);
void KeyProcess(void);

int _tmain(int argc, _TCHAR* argv[])
{
	timeBeginPeriod(1);

	LoadConfig();
	
	// 콘솔창 크기
	system("mode con: lines=6 cols=80");

	time_t startTime = time(NULL);
	tm t;
	localtime_s(&t, &startTime);

	CGameServer GameServer(g_Config.iClientMax);
	g_pGameServer = &GameServer;
	g_pGameServer->Start();

	while (true)
	{
		KeyProcess();

		wprintf_s(L"SERVER ON TIME : [%04d-%02d-%02d %02d:%02d:%02d]\n", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
		wprintf_s(L"=========================================================\n");
		wprintf_s(L"Accept Total\t: %I64u\n", g_pGameServer->_iAcceptTotal);
		wprintf_s(L"AcceptTPS\t: %d\n\n", g_pGameServer->_iAcceptTPS);
		//wprintf_s(L"");
		//wprintf_s(L"");

		Sleep(998);
	}

	timeEndPeriod(1);
	return 0;
}

bool LoadConfig(void)
{
	CConfigParser parser;
	wchar_t szKey[128];

	if (!parser.OpenConfigFile(L"GameServerConfig.ini"))
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found config file");
		return false;
	}
	else
	{
		///////////////////////////////////////////////
		// NETWORK Block
		///////////////////////////////////////////////
		wsprintf(szKey, L"NETWORK");
		if (!parser.MoveConfigBlock(szKey))
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
			return false;
		}
		else
		{
			wsprintf(szKey, L"NET_BIND_IP");
			if (!parser.GetValue(szKey, g_Config.szNetBindIP, 16))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"NET_BIND_PORT");
			if (!parser.GetValue(szKey, &g_Config.iNetBindPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"LAN_BIND_IP");
			if (!parser.GetValue(szKey, g_Config.szNetBindIP, 16))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"LAN_BIND_PORT");
			if (!parser.GetValue(szKey, &g_Config.iLanBindPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_IP");
			if (!parser.GetValue(szKey, g_Config.szLoginServerIP, 16))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_PORT");
			if (!parser.GetValue(szKey, &g_Config.iLoginServerPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"MONITORING_SERVER_IP");
			if (!parser.GetValue(szKey, g_Config.szMonitoringServerIP, 16))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"MONITORING_SERVER_PORT");
			if (!parser.GetValue(szKey, &g_Config.iMonitoringServerPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}
		}

		///////////////////////////////////////////////
		// SYSTEM Block
		///////////////////////////////////////////////
		wsprintf(szKey, L"SYSTEM");
		if (!parser.MoveConfigBlock(szKey))
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
			return false;
		}
		else
		{
			wsprintf(szKey, L"CLIENT_MAX");
			if (!parser.GetValue(szKey, &g_Config.iClientMax))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"PACKET_CODE");
			if (!parser.GetValue(szKey, &g_Config.iPacketCode))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"PACKET_KEY1");
			if (!parser.GetValue(szKey, &g_Config.iPacketKey1))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"PACKET_KEY2");
			if (!parser.GetValue(szKey, &g_Config.iPacketKey2))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			CPacket::SetEncryptCode((BYTE)g_Config.iPacketCode, (BYTE)g_Config.iPacketKey1, (BYTE)g_Config.iPacketKey2);
		}

		///////////////////////////////////////////////
		// LOG Block
		///////////////////////////////////////////////
		wsprintf(szKey, L"LOG");
		if (!parser.MoveConfigBlock(szKey))
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
			return false;
		}
		else
		{
			wsprintf(szKey, L"LOG_LEVEL");
			if (!parser.GetValue(szKey, &g_Config.iLogLevel))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"PRINT_CONSOLE");
			if (!parser.GetValue(szKey, &g_Config.bPrintConsole))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"PRINT_FILE");
			if (!parser.GetValue(szKey, &g_Config.bPrintFile))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"PRINT_DATABASE");
			if (!parser.GetValue(szKey, &g_Config.bPrintDatabase))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			SYSLOG_SETDIRECTORY(L"LOG");

			LOG::iLogLevel = g_Config.iLogLevel;
			LOG::bConsole = g_Config.bPrintConsole;
			LOG::bFile = g_Config.bPrintFile;
			LOG::bDatabase = g_Config.bPrintDatabase;
		}
	}

	return true;
}

void KeyProcess(void)
{
	char pressedKey;

	if (0 != _kbhit())
	{
		pressedKey = _getch();

		switch (pressedKey)
		{
			case 'S':
			case 's':
			{
				g_pGameServer->Start();
				break;
			}

			case 'Q':
			case 'q':
			{
				g_pGameServer->Stop();
				break;
			}
		}
	}

	return;
}