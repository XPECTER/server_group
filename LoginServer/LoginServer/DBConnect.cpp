#include "stdafx.h"
#include "DBConnect.h"


CDBConnector::CDBConnector()
{
	mysql_init(&this->_connection);
	this->_pConnection = nullptr;
	this->_pResult = nullptr;
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

bool CDBConnector::Query(char *szQuery, ...)
{
	char query[1024] = { 0, };
	va_list vl;

	va_start(vl, szQuery);
	vsprintf_s(query, 1024, szQuery, vl);
	va_end(vl);

	//sprintf_s(this->_szLastQuery, 1024, query);
	if (0 == mysql_query(&this->_connection, query))
	{
		this->_pResult = mysql_store_result(&this->_connection);
		return true;
	}
	else
		return false;
}

MYSQL_ROW CDBConnector::FetchRow(void)
{
	return mysql_fetch_row(this->_pResult);
}

void CDBConnector::FreeResult(void)
{
	mysql_free_result(this->_pResult);
	return;
}

int CDBConnector::GetLastError(void)
{
	return mysql_errno(this->_pConnection);
}

