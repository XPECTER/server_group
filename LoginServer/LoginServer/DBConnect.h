#pragma once

class CDBConnector
{
public:
	CDBConnector();
	virtual ~CDBConnector();

	bool Connect(wchar_t *szConnectIP, wchar_t *szUser, wchar_t *szPassword, wchar_t *szDBName, int iPort);

	void Query(char *szQuery);
	MYSQL_ROW FetchRow(void);

	void Lock();
	void UnLock();

	virtual bool ReadDB(en_DB_ACTION_TYPE, void *pIn, void *pOut) = 0;
	virtual bool WriteDB(en_DB_ACTION_TYPE, void *pIn) = 0;

private:
	MYSQL _connection;
	MYSQL *_pConnection;

	MYSQL_RES *_pResult;
	SRWLOCK _srwLock;
};

class AccountDB : public CDBConnector
{
public:
	virtual bool ReadDB(en_DB_ACTION_TYPE, void *pIn, void *pOut) override;
	virtual bool WriteDB(en_DB_ACTION_TYPE, void *pIn) override;
};