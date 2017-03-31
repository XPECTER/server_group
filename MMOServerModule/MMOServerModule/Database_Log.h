#pragma once

class LogDB : public CDBConnectorTLS
{
public:
	LogDB(char *szConnectIP, char *szConnectUser, char *szConnectPass, char *szConnectDBName, int iConnectPort, CGameServer *pGameServer);
	virtual ~LogDB();

	bool QueryDB(WORD type, WORD code, __int64 AccountNo, wchar_t *szServerName, __int64 param1, __int64 param2, __int64 param3, __int64 param4, wchar_t *szMessage);

private:
	CGameServer *_pGameServer;
};