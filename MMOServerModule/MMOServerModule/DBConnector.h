#pragma once

class CDBConnector
{
public:
	CDBConnector();
	virtual ~CDBConnector();

	bool Connect(wchar_t *szDBIP, wchar_t *szUser, wchar_t *szPassword, wchar_t *szDBName, int iDBPort);
private:
	MYSQL auth_accountdb;
	MYSQL auth_gamedb;
	MYSQL auth_logdb;

	MYSQL game_accountdb;
	MYSQL game_gamedb;
	MYSQL game_logdb;

	MYSQL db_accountdb;
	MYSQL db_gamedb;
	MYSQL db_logdb;

};