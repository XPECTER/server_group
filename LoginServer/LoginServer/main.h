#pragma once
#define dfSERVER_NAME_LEN 32
#define dfSERVER_ADDR_LEN 16
#define dfSERVER_LINK_NUM 10


struct st_SERVER_LINK_INFO
{
	wchar_t _serverName[dfSERVER_NAME_LEN];

	wchar_t _gameServerIP[dfSERVER_ADDR_LEN];
	int		_gameServerPort;
	wchar_t _chatServerIP[dfSERVER_ADDR_LEN];
	int		_chatServerPort;

	wchar_t _gameServerIP_Dummy[dfSERVER_ADDR_LEN];
	int		_gameServerPort_Dummy;
	wchar_t _chatServerIP_Dummy[dfSERVER_ADDR_LEN];
	int		_chatServerPort_Dummy;
};

class CConfigData
{
public:
	////////////////////////////////////////
	// Network Block
	////////////////////////////////////////
	wchar_t _szServerGroupName[dfSERVER_NAME_LEN];
	int _iNetServerWorkerThreadNum;
	int _iLanServerWorkerThreadNum;

	wchar_t _szLoginServerNetBindIP[dfSERVER_ADDR_LEN];
	int		_iLoginServerNetBindPort;
	bool	_bLoginServerNetNagleOpt;

	wchar_t _szLoginServerLanBindIP[dfSERVER_ADDR_LEN];
	int		_iLoginServerLanBindPort;
	bool	_bLoginServerLanNagleOpt;

	wchar_t _szMonitoringServerIP[dfSERVER_ADDR_LEN];
	int		_iMonitoringServerPort;
	bool	_bMonitoringServerNagleOpt;
	

	////////////////////////////////////////
	// System Block
	////////////////////////////////////////
	int _LoginServerNetClientMax;
	int _LoginServerLanClientMax;

	int _iPacketCode;
	int _iPacketKey1;
	int _iPacketKey2;

	wchar_t _szServerLinkConfigFileName[128];


	////////////////////////////////////////
	// Log Block
	////////////////////////////////////////
	int		_iLogLevel;
	bool	_bPrintConsole;
	bool	_bPrintFile;
	bool	_bPrinfDatabase;

	
	////////////////////////////////////////
	// Database Block
	////////////////////////////////////////
	// AccountDB
	wchar_t _szAccountIP[16];
	int		_iAccountPort;
	wchar_t _szAccountUser[32];
	wchar_t _szAccountPassword[32];
	wchar_t _szAccountDBName[32];

	// Login Log DB
	wchar_t _szLogIP[16];
	int		_iLogPort;
	wchar_t _szLogUser[32];
	wchar_t _szLogPassword[32];
	wchar_t _szLogDBName[32];


	////////////////////////////////////////
	// ServerLink Block
	////////////////////////////////////////
	st_SERVER_LINK_INFO _serverLinkInfo[dfSERVER_LINK_NUM];

public:
	/*static CConfigData *Instance;

	CConfigData* GetInstance(void)
	{
		if (nullptr == this->Instance)
		{
			this->Instance = new CConfigData;
			return this->Instance;
		}
		else
		{
			return this->Instance;
		}
	}*/

	// 서버 번호를 돌려주는 함수가 필요할지도
	int GetServerNum(wchar_t *serverName)
	{
		for (int i = 0; i < dfSERVER_LINK_NUM; ++i)
		{
			if (0 == wcscmp(this->_serverLinkInfo[i]._serverName, serverName))
			{
				return i + 1;
			}
		}

		return 0;
	}
};

bool LoadConfigData(void);


extern CCrashDump crashDump;
extern CConfigData g_ConfigData;
extern AccountDB g_AccountDB;