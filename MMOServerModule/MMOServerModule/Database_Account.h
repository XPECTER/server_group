#pragma once

class AccountDB : public CDBConnectorTLS
{
public:
	AccountDB(char *szConnectIP, char *szConnectUser, char *szConnectPass, char *szConnectDBName, int iConnectPort, CGameServer *pGameServer);
	virtual ~AccountDB();

	void QueryDB(en_DB_ACTION_TYPE type, PVOID pIn, PVOID pOut);

private:
	CGameServer *_pGameServer;
};