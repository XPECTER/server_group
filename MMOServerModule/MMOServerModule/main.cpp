// MMOServerModule.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "GameServer.h"

CMemoryPoolTLS<CPacket> CPacket::PacketPool(300, true);
CConfig g_Config;
CGameServer *g_pGameServer = nullptr;


bool LoadConfig(void);
void KeyProcess(void);


int _tmain(int argc, _TCHAR* argv[])
{
	//////////////////////////////////////////////////////////
	// 필드 테스트 코드
	/*CField<CLIENT_ID> field(600, 200);

	field.AddTileObject(1, 0, 0);
	field.AddTileObject(2, 0, 0);
	field.AddTileObject(3, 0, 0);
	field.AddTileObject(4, 0, 0);

	field.DelTileObject(3, 0, 0);

	std::list<CLIENT_ID> list;
	field.GetTileObject(0, 0, &list);*/

	// 점프 포인트 서치 테스트 코드
	CJumpPointSearch jps(600, 200);		// 서버에서 사용할 크기이다. 클라이언트 맵의 X2배씩
	jps.JumpPointSearch_Init();
	jps.LoadTextMap(L"Map.txt");
	int out = 0;
	jps.FindPath(4, 20, 4, 10, NULL, &out);


	//////////////////////////////////////////////////////////
	timeBeginPeriod(1);

	unsigned __int64 iLoop = 0;

	if (!LoadConfig())
	{
		return 0;
	}
	
	// 콘솔창 크기
	system("mode con: lines=18 cols=80");

	time_t startTime = time(NULL);
	tm t;
	localtime_s(&t, &startTime);

	CGameServer GameServer(g_Config.iClientMax);
	g_pGameServer = &GameServer;
	//g_pGameServer->Start(g_Config.szNetBindIP, g_Config.iNetBindPort, g_Config.bNetNagleOpt, g_Config.iNetThreadNum);

	while (true)
	{
		g_pGameServer->Start();
		KeyProcess();

		wprintf_s(L"SERVER ON TIME : [%04d-%02d-%02d %02d:%02d:%02d]\tLoop : %I64u\n", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, iLoop);
		wprintf_s(L"=========================================================\n");
		wprintf_s(L"Accept Total\t\t: %I64u\n", g_pGameServer->_iAcceptTotal);
		wprintf_s(L"AcceptTPS\t\t: %d\n", g_pGameServer->_iAcceptTPS);
		wprintf_s(L"SessionCount\t\t: %d\n", g_pGameServer->_iTotalSessionCounter);
		wprintf_s(L"=========================================================\n");
		wprintf_s(L"AuthTh Loop TPS\t\t: %d\n", g_pGameServer->_iAuthThLoopTPS);
		wprintf_s(L"Session in AuthMode\t: %d\n", g_pGameServer->_iAuthThSessionCounter);
		wprintf_s(L"=========================================================\n");
		wprintf_s(L"GameTh Loop TPS\t\t: %d\n", g_pGameServer->_iGameThLoopTPS);
		wprintf_s(L"Session in GameMode\t: %d\n", g_pGameServer->_iGameThSessionCounter);
		wprintf_s(L"=========================================================\n");
		wprintf_s(L"RecvPacketCount\t\t: %d\n", g_pGameServer->_iRecvPacketTPS);
		wprintf_s(L"SendPacketCount\t\t: %d\n", g_pGameServer->_iSendPacketTPS);
		wprintf_s(L"=========================================================\n");
		wprintf_s(L"Packet Pool Chunk Size\t: %d\n", CPacket::PacketPool.GetAllocCount());
		wprintf_s(L"Using Packet\t\t: %d\n", CPacket::PacketPool.GetUseCount());

		iLoop++;
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
			wsprintf(szKey, L"SERVER_GROUP_NAME");
			if (!parser.GetValue(szKey, g_Config.szServerGroupName, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

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

			wsprintf(szKey, L"NET_THREAD_NUM");
			if (!parser.GetValue(szKey, &g_Config.iNetThreadNum))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}
			
			wsprintf(szKey, L"NET_NAGLE_OPT");
			if (!parser.GetValue(szKey, &g_Config.bNetNagleOpt))
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

			wsprintf(szKey, L"LAN_THREAD_NUM");
			if (!parser.GetValue(szKey, &g_Config.iLanThreadNum))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"LAN_NAGLE_OPT");
			if (!parser.GetValue(szKey, &g_Config.bLanNagleOpt))
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

			wsprintf(szKey, L"AGENT_SERVER_IP");
			if (!parser.GetValue(szKey, g_Config.szAgentServerIP, 16))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"AGENT_SERVER_PORT");
			if (!parser.GetValue(szKey, &g_Config.iAgentServerPort))
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

		///////////////////////////////////////////////
		// Database Block
		///////////////////////////////////////////////
		wsprintf(szKey, L"DATABASE");
		if (!parser.MoveConfigBlock(szKey))
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
			return false;
		}
		else
		{
			// AccountDB
			wsprintf(szKey, L"ACCOUNT_IP");
			if (!parser.GetValue(szKey, g_Config.szAccountDBIP, 16))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"ACCOUNT_PORT");
			if (!parser.GetValue(szKey, &g_Config.iAccountDBPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"ACCOUNT_USER");
			if (!parser.GetValue(szKey, g_Config.szAccountDBUser, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"ACCOUNT_PASSWORD");
			if (!parser.GetValue(szKey, g_Config.szAccountDBPassword, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"ACCOUNT_DBNAME");
			if (!parser.GetValue(szKey, g_Config.szAccountDBName, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			// GameDB
			wsprintf(szKey, L"GAME_IP");
			if (!parser.GetValue(szKey, g_Config.szGameDBIP, 16))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"GAME_PORT");
			if (!parser.GetValue(szKey, &g_Config.iGameDBPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"GAME_USER");
			if (!parser.GetValue(szKey, g_Config.szGameDBUser, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"GAME_PASSWORD");
			if (!parser.GetValue(szKey, g_Config.szGameDBPassword, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"GAME_DBNAME");
			if (!parser.GetValue(szKey, g_Config.szGameDBName, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			// LogDB
			wsprintf(szKey, L"LOG_IP");
			if (!parser.GetValue(szKey, g_Config.szLogDBIP, 16))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"LOG_PORT");
			if (!parser.GetValue(szKey, &g_Config.iLogDBPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"LOG_USER");
			if (!parser.GetValue(szKey, g_Config.szLogDBUser, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"LOG_PASSWORD");
			if (!parser.GetValue(szKey, g_Config.szLogDBPassword, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}

			wsprintf(szKey, L"LOG_DBNAME");
			if (!parser.GetValue(szKey, g_Config.szLogDBName, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not found block : %s", szKey);
				return false;
			}
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
				//g_pGameServer->Start(g_Config.szNetBindIP, g_Config.iNetBindPort, g_Config.bNetNagleOpt, g_Config.iNetThreadNum);
				g_pGameServer->Start();
				break;
			}

			case 'Q':
			case 'q':
			{
				//g_pGameServer->Stop();
				break;
			}
		}
	}

	return;
}