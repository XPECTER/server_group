#include "stdafx.h"
#include "DBConnect.h"

CDBConnector::CDBConnector()
{
	mysql_init(&this->_connection);
	this->_pConnection = nullptr;
	this->_pResult = nullptr;

	InitializeSRWLock(&this->_srwLock);
}

CDBConnector::~CDBConnector()
{
	mysql_close(this->_pConnection);
}

bool CDBConnector::Connect(wchar_t *szConnectIP, wchar_t *szUser, wchar_t *szPassword, wchar_t *szDBName, int iPort)
{
	size_t i;
	char szConvertIP[16] = { 0, };
	char szConvertUser[32] = { 0, };
	char szConvertPassword[32] = { 0, };
	char szConvertDBName[32] = { 0, };
	
	wcstombs_s(&i, szConvertIP, 16, szConnectIP, 16);
	wcstombs_s(&i, szConvertUser, 32, szUser, 32);
	wcstombs_s(&i, szConvertPassword, szPassword, 32);
	wcstombs_s(&i, szConvertDBName, szDBName, 32);

	this->_pConnection = mysql_real_connect(&this->_connection, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);

	if (NULL == this->_pConnection)
		return false;
	else
		return true;
}

void CDBConnector::Query(char *szQuery)
{
	mysql_query(this->_pConnection, szQuery);
	this->_pResult = mysql_store_result(this->_pConnection);
	return;
}

MYSQL_ROW CDBConnector::FetchRow(void)
{
	return mysql_fetch_row(this->_pResult);
}

void CDBConnector::Lock(void)
{
	AcquireSRWLockExclusive(&this->_srwLock);
	return;
}

void CDBConnector::UnLock(void)
{
	ReleaseSRWLockExclusive(&this->_srwLock);
	return;
}

bool AccountDB::ReadDB(en_DB_ACTION_TYPE type, void *pIn, void *pOut)
{
	char szQuery[2048] = { 0, };
	MYSQL_ROW row;

	switch (type)
	{
		case enDB_ACCOUNT_READ_LOGIN_SESSION:
			sprintf_s(szQuery, 2048, "SELECT * FROM `accountdb`.`status`;");
			this->Query(szQuery);
			row = this->FetchRow();
			break;

		case enDB_ACCOUNT_READ_WHITE_IP:
			break;

		case enDB_ACCOUNT_READ_RESET_STATUS_ALL:
			sprintf_s(szQuery, 2048, "UPDATE `accountdb`.`status` SET status = 0;");
			this->Query(szQuery);
			break;
	}

	return true;
}

bool AccountDB::WriteDB(en_DB_ACTION_TYPE type, void *pIn)
{
	return true;
}