#pragma once
#define dfSERVER_ADDR_LEN 16
#define dfSERVER_NAME_LEN 32

DWORD KeyProcess(void);
bool LoadConfigData(void);
void PrintConfig(void);

class CConfigData
{
public:
	////////////////////////////////////////
	// Network Block
	////////////////////////////////////////
	wchar_t _szServerGroupName[dfSERVER_NAME_LEN];
	
	int		_iThreadNum;

	// ChatServer Config
	wchar_t	_szChatServerBindIP[dfSERVER_NAME_LEN];
	int		_iChatServerBindPort;
	bool	_bChatServerNagleOpt;

	// LoginServer Config
	wchar_t	_szLoginServerIP[dfSERVER_NAME_LEN];
	int		_iLoginServerPort;
	bool	_bLoginServerNagleOpt;

	// MonitoringServer Config
	wchar_t	_szMonitoringServerIP[dfSERVER_NAME_LEN];
	int		_iMonitoringServerPort;
	bool	_bMonitoringServerNagleOpt;


	////////////////////////////////////////
	// System Block
	////////////////////////////////////////
	int		_iClientMax;
	int		_iPacketCode;
	int		_iPacketKey1;
	int		_iPacketKey2;


	////////////////////////////////////////
	// Log Block
	////////////////////////////////////////
	int		_iLogLevel;
	bool	_bPrintConsole;
	bool	_bPrintFile;
	bool	_bPrinfDatabase;
};

extern CCrashDump crashDump;
extern CConfigData g_ConfigData;