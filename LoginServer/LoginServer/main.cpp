// LoginServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "main.h"
#include "LoginServer.h"
#include "LanServer_Login.h"


CMemoryPoolTLS<CPacket> CPacket::PacketPool(300, true);
BYTE CPacket::_packetCode = 0;
BYTE CPacket::_packetKey_1 = 0;
BYTE CPacket::_packetKey_2 = 0;

long CCrashDump::_DumpCount = 0;
CCrashDump crashDump;

CConfigData g_ConfigData;
CLoginServer loginServer;

int _tmain(int argc, _TCHAR* argv[])
{
	LoadConfigData();
	loginServer.Start();

	tm t;
	time_t startTime = time(NULL);
	localtime_s(&t, &startTime);

	while (true)
	{
		Sleep(998);
		system("cls");
		wprintf_s(L"SERVER ON TIME : [%04d-%02d-%02d %02d:%02d:%02d]", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
		wprintf_s(L"\n===============================================================\n");

		wprintf_s(L"Session Count : %d\n", loginServer.GetClientCount());
		wprintf_s(L"Packet Pool Alloc Size : %d\n", CPacket::PacketPool.GetAllocCount());
		wprintf_s(L"Using Packet : %d\n\n", CPacket::PacketPool.GetUseCount());

		wprintf_s(L"Player Pool Alloc Size : %d\n", loginServer.GetPlayerAllocCount());
		wprintf_s(L"Player Count : %d\n\n", loginServer.GetPlayerUseCount());

		wprintf_s(L"Accept Total : %I64u\n", loginServer._iAcceptTotal);
		wprintf_s(L"Accept TPS : %d\n", loginServer._iAcceptTPS);
		
		wprintf_s(L"RecvPacket TPS : %d\n", loginServer._iRecvPacketTPS);
		wprintf_s(L"SendPacket TPS : %d\n\n", loginServer._iSendPacketTPS);

		wprintf_s(L"Login Success TPS : %d\n", loginServer._Monitor_LoginSuccessTPS);
		wprintf_s(L"Login Wait : %d\n\n", loginServer._Monitor_LoginWait);

		wprintf_s(L"OnSend Call Count : %I64u\n", loginServer._OnSendCallCount);
		wprintf_s(L"LanServer SendPacket Count : %d\n", loginServer.GetLanSendPacketCount());
		wprintf_s(L"LanServer RecvPacket Count : %d\n", loginServer.GetLanSendPacketCount());
		/*wprintf_s(L"Login Process Time Max : %d\n", loginServer._Monitor_LoginProcessTime_Max);
		wprintf_s(L"Login Process Time Min : %d\n", loginServer._Monitor_LoginProcessTime_Min);
		wprintf_s(L"Login Process Time Avr : %f\n\n", loginServer.GetLoginProcessAvg());*/
	}

	return 0;
}


bool LoadConfigData(void)
{
	CConfigParser parser;
	wchar_t szBlock[32] = { 0, };
	wchar_t szKey[32] = { 0, };

	if (!parser.OpenConfigFile(L"LoginServerConfig.ini"))
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
			if (!parser.GetValue(szKey, g_ConfigData._szServerGroupName, dfSERVER_NAME_LEN))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_NET_WORKER_THREAD_NUM");
			if (!parser.GetValue(szKey, &g_ConfigData._iNetServerWorkerThreadNum))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_LAN_WORKER_THREAD_NUM");
			if (!parser.GetValue(szKey, &g_ConfigData._iLanServerWorkerThreadNum))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_NET_BIND_IP");
			if (!parser.GetValue(szKey, g_ConfigData._szLoginServerNetBindIP, dfSERVER_ADDR_LEN))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_NET_BIND_PORT");
			if (!parser.GetValue(szKey, &g_ConfigData._iLoginServerNetBindPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_NET_NAGLE_OPT");
			if (!parser.GetValue(szKey, &g_ConfigData._bLoginServerNetNagleOpt))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_LAN_BIND_IP");
			if (!parser.GetValue(szKey, g_ConfigData._szLoginServerLanBindIP, dfSERVER_ADDR_LEN))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_LAN_BIND_PORT");
			if (!parser.GetValue(szKey, &g_ConfigData._iLoginServerLanBindPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_LAN_NAGLE_OPT");
			if (!parser.GetValue(szKey, &g_ConfigData._bLoginServerLanNagleOpt))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"MONITORING_SERVER_IP");
			if (!parser.GetValue(szKey, g_ConfigData._szMonitoringServerIP, dfSERVER_ADDR_LEN))
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
			wsprintf(szKey, L"LOGIN_SERVER_NET_CLIENT_MAX");
			if (!parser.GetValue(szKey, &g_ConfigData._LoginServerNetClientMax))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOGIN_SERVER_LAN_CLIENT_MAX");
			if (!parser.GetValue(szKey, &g_ConfigData._LoginServerLanClientMax))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"SERVER_LINK_CNF");
			if (!parser.GetValue(szKey, g_ConfigData._szServerLinkConfigFileName, 128))
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

		wsprintf(szBlock, L"DATABASE");
		if (!parser.MoveConfigBlock(szBlock))
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s BLOCK in Config file", szBlock);
			return false;
		}
		else
		{
			wsprintf(szKey, L"ACCOUNT_IP");
			if (!parser.GetValue(szKey, g_ConfigData._szAccountIP, 16))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"ACCOUNT_PORT");
			if (!parser.GetValue(szKey, &g_ConfigData._iAccountPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"ACCOUNT_USER");
			if (!parser.GetValue(szKey, g_ConfigData._szAccountUser, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"ACCOUNT_PASSWORD");
			if (!parser.GetValue(szKey, g_ConfigData._szAccountPassword, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"ACCOUNT_DBNAME");
			if (!parser.GetValue(szKey, g_ConfigData._szAccountDBName, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOG_IP");
			if (!parser.GetValue(szKey, g_ConfigData._szLogIP, 16))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOG_PORT");
			if (!parser.GetValue(szKey, &g_ConfigData._iLogPort))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOG_USER");
			if (!parser.GetValue(szKey, g_ConfigData._szLogUser, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOG_PASSWORD");
			if (!parser.GetValue(szKey, g_ConfigData._szLogPassword, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}

			wsprintf(szKey, L"LOG_DBNAME");
			if (!parser.GetValue(szKey, g_ConfigData._szLogDBName, 32))
			{
				SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"Not exist %s Key", szKey);
				return false;
			}
		}
	}

	if (!parser.OpenConfigFile(g_ConfigData._szServerLinkConfigFileName))
	{
		SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"%s ConfigFile Open Failed", g_ConfigData._szServerLinkConfigFileName);
		return false;
	}
	else
	{
		wsprintf(szBlock, L"SERVER_LINK");
		if (!parser.MoveConfigBlock(szBlock))
		{
			SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"LanServer_Login not exist %s block", szBlock);
			return false;
		}
		else
		{
			for (int i = 0; i < dfSERVER_LINK_NUM; ++i)
			{
				wsprintf(szKey, L"%d_NAME", i + 1);
				if (!parser.GetValue(szKey, g_ConfigData._serverLinkInfo[i]._serverName, dfSERVER_NAME_LEN))
					break;

				wsprintf(szKey, L"%d_GAMEIP", i + 1);
				if (!parser.GetValue(szKey, g_ConfigData._serverLinkInfo[i]._gameServerIP, dfSERVER_NAME_LEN))
				{
					SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"[LanServer_Login] not exist %s key", szKey);
					return false;
				}

				wsprintf(szKey, L"%d_GAMEPORT", i + 1);
				if (!parser.GetValue(szKey, &g_ConfigData._serverLinkInfo[i]._gameServerPort))
				{
					SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"[LanServer_Login] not exist %s key", szKey);
					return false;
				}

				wsprintf(szKey, L"%d_CHATIP", i + 1);
				if (!parser.GetValue(szKey, g_ConfigData._serverLinkInfo[i]._chatServerIP, dfSERVER_NAME_LEN))
				{
					SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"[LanServer_Login] not exist %s key", szKey);
					return false;
				}

				wsprintf(szKey, L"%d_CHATPORT", i + 1);
				if (!parser.GetValue(szKey, &g_ConfigData._serverLinkInfo[i]._chatServerPort))
				{
					SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"[LanServer_Login] not exist %s key", szKey);
					return false;
				}

				wsprintf(szKey, L"%d_GAMEIP_DUMMY", i + 1);
				if (!parser.GetValue(szKey, g_ConfigData._serverLinkInfo[i]._gameServerIP_Dummy, dfSERVER_NAME_LEN))
				{
					SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"[LanServer_Login] not exist %s key", szKey);
					return false;
				}

				wsprintf(szKey, L"%d_GAMEPORT_DUMMY", i + 1);
				if (!parser.GetValue(szKey, &g_ConfigData._serverLinkInfo[i]._gameServerPort_Dummy))
				{
					SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"[LanServer_Login] not exist %s key", szKey);
					return false;
				}

				wsprintf(szKey, L"%d_CHATIP_DUMMY", i + 1);
				if (!parser.GetValue(szKey, g_ConfigData._serverLinkInfo[i]._chatServerIP_Dummy, dfSERVER_NAME_LEN))
				{
					SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"[LanServer_Login] not exist %s key", szKey);
					return false;
				}

				wsprintf(szKey, L"%d_CHATPORT_DUMMY", i + 1);
				if (!parser.GetValue(szKey, &g_ConfigData._serverLinkInfo[i]._chatServerPort_Dummy))
				{
					SYSLOG(L"SYSTEM", LOG::LEVEL_ERROR, L"[LanServer_Login] not exist %s key", szKey);
					return false;
				}
			}
		}
	}

	return true;
}

