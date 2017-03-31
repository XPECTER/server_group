#pragma once

#pragma comment (lib, "mysqlclient.lib")

#include <my_global.h>
#include <mysql.h>

#include "APIHook.h"
#include "CrashDump.h"
#include "LockFreeStack.h"
#include "Log.h"


class CDBConnectorTLS
{
private:
	class CConnector
	{
	public:
		CConnector();
		~CConnector();

		void ConnectDB(char *szDBIP, char *szDBUser, char *szDBPass, char *szDBName, int iDBPort);
		bool Query(char *szQuery);

		MYSQL_ROW FetchRow(void);
		void FreeResult(void);
		int GetLastError(void);

	private:
		MYSQL _connector;
		MYSQL *_pConnector;
		MYSQL_RES *_pResult;
	};

public:
	CDBConnectorTLS(char *szConnectIP, char *szConnectUser, char *szConnectPass, char *szConnectDBName, int iConnectPort);
	virtual ~CDBConnectorTLS();

protected:
	bool Query(char *szQuery, ...);
	MYSQL_ROW FetchRow(void);
	int GetLastError(void);
	void FreeResult(void);

private:
	CConnector* GetConnector(void);

private:
	// TLS Index
	int _iTLSIndex;

	// DB Connector Queue
	CLockFreeStack<CConnector *> _connectorStack;

	// connect info
	char _szConnectIP[16];
	char _szConnectUser[32];
	char _szConnectPass[32];
	char _szConnectDBName[32];
	int _iConnectPort;
};
