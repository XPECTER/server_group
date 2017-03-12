#pragma once

//class CDBConnector
//{
//public:
//	CDBConnector();
//	virtual ~CDBConnector();
//
//	bool Connect(wchar_t *szDBIP, wchar_t *szUser, wchar_t *szPassword, wchar_t *szDBName, int iDBPort);
//
//public:
//	bool auth_DBWrite(en_DB_ACTION_TYPE type, void *pIn, void *pOut);
//	bool game_DBWrite(en_DB_ACTION_TYPE type, void *pIn, void *pOut);
//
//private:
//	MYSQL auth_accountdb;
//	MYSQL auth_gamedb;
//	MYSQL auth_logdb;
//
//	MYSQL game_accountdb;
//	MYSQL game_gamedb;
//	MYSQL game_logdb;
//
//	MYSQL db_accountdb;
//	MYSQL db_gamedb;
//	MYSQL db_logdb;
//
//};




class CDBConnectorTLS
{
private:
	class CConnector
	{
	public:
		CConnector();
		~CConnector();

		void ConnectDB(void);
		bool Query(int iDBType, char *szQuery);

		MYSQL_ROW FetchRow(void);
		void FreeResult(void);
		int GetLastError(void);

	private:
		MYSQL _connection_AccountDB;
		MYSQL _connection_GameDB;
		MYSQL _connection_LogDB;

		MYSQL *_pConnection_AccountDB;
		MYSQL *_pConnection_GameDB;
		MYSQL *_pConnection_LogDB;

		MYSQL_RES *_pResult_AccountDB;
		MYSQL_RES *_pResult_GameDB;
		MYSQL_RES *_pResult_LogDB;

		int _dbType;
	};

public:
	CDBConnectorTLS();
	virtual ~CDBConnectorTLS();

protected:
	bool Query(int iDBType, char *szQuery, ...);
	MYSQL_ROW FetchRow(void);
	int GetLastError(void);
	void FreeResult(void);

private:
	CConnector* GetConnector(void);

private:
	// TLS Index
	int _iTLSIndex;

	// DB Connector Queue
	CLockFreeQueue<CConnector *> _connectorQueue;
};

class CDatabase : public CDBConnectorTLS
{
public:
	CDatabase();
	virtual ~CDatabase();

	void Query_AccountDB(en_DB_ACTION_TYPE type, PVOID pIn, PVOID pOut);
	void Query_GameDB(en_DB_ACTION_TYPE type, PVOID pIn, PVOID pOut);
};