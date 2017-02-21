#pragma once

class CDBConnector
{
public:
	CDBConnector();
	virtual ~CDBConnector();

	bool Connect(wchar_t *szConnectIP, wchar_t *szUser, wchar_t *szPassword, wchar_t *szDBName, int iPort);

	bool Query(char *szQuery, ...);
	MYSQL_ROW FetchRow(void);
	void FreeResult(void);
	int GetLastError(void);

public:
	char _szLastQuery[1024];

private:
	MYSQL _connection;
	MYSQL *_pConnection;
	MYSQL_RES *_pResult;
};



