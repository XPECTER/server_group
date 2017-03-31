#pragma once

class GameDB : public CDBConnectorTLS
{
public:
	GameDB(char *szConnectIP, char *szConnectUser, char *szConnectPass, char *szConnectDBName, int iConnectPort, CGameServer *pGameServer);
	virtual ~GameDB();

	void QueryDB(en_DB_ACTION_TYPE type, PVOID pIn, PVOID pOut);

private:
	CGameServer *_pGameServer;
};