// MMOServerModule.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

CMemoryPoolTLS<CPacket> CPacket::PacketPool(300, true);
CConfig g_Config;

bool LoadConfig(void);

int _tmain(int argc, _TCHAR* argv[])
{
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
		}
	}
}