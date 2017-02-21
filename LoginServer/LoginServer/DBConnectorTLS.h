#pragma once

class CDBConnectorTLS : public CDBConnector
{
public:
	CDBConnectorTLS();
	virtual ~CDBConnectorTLS();

	bool Connect();

	bool Query(char *szQuery, ...);
	MYSQL_ROW FetchRow(void);
	void FreeResult(void);
	int GetLastError(void);

private:
	CDBConnector *GetInstance();

private:
	int _iTlsIndex;
	CLockFreeStack<CDBConnector *> _stack;
};


class AccountDB : public CDBConnectorTLS
{
public:
	AccountDB();
	~AccountDB();

	bool ReadDB(en_DB_ACTION_TYPE, void *pIn, void *pOut);
	bool WriteDB(en_DB_ACTION_TYPE, void *pIn);
};