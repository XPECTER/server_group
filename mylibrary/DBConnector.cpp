#include "DBConnector.h"


CDBConnectorTLS::CConnector::CConnector()
{
	mysql_init(&this->_connector);
}

CDBConnectorTLS::CConnector::~CConnector()
{
	mysql_close(this->_pConnector);
}

void CDBConnectorTLS::CConnector::ConnectDB(char *szDBIP, char *szDBUser, char *szDBPass, char *szDBName, int iDBPort)
{
	// Mysql library 버전이 다를 경우 내 꺼 아닌 메모리를 침범할 수 있다.
	this->_pConnector = mysql_real_connect(&this->_connector, szDBIP, szDBUser, szDBPass, szDBName, iDBPort, (char *)NULL, 0);
	if (NULL == this->_pConnector)
		SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"connect failed. %d", this->GetLastError());
	
	return;
}

CDBConnectorTLS::CConnector* CDBConnectorTLS::GetConnector(void)
{
	CConnector *pConn = (CConnector *)TlsGetValue(this->_iTLSIndex);

	if (NULL == pConn)
	{
		pConn = new CConnector;
		pConn->ConnectDB(this->_szConnectIP, this->_szConnectUser, this->_szConnectPass, this->_szConnectDBName, this->_iConnectPort);

		TlsSetValue(this->_iTLSIndex, (LPVOID)pConn);
		this->_connectorStack.Push(pConn);
	}

	return pConn;
}

bool CDBConnectorTLS::CConnector::Query(char *szQuery)
{
	int query_stat = 0;
	query_stat = mysql_query(this->_pConnector, szQuery);

	if (0 == query_stat)
	{
		this->_pResult = mysql_store_result(this->_pConnector);
		return true;
	}
	else
		return false;
}

MYSQL_ROW CDBConnectorTLS::CConnector::FetchRow(void)
{
	MYSQL_ROW row = NULL;
	return mysql_fetch_row(this->_pResult);
}

void CDBConnectorTLS::CConnector::FreeResult(void)
{
	mysql_free_result(this->_pResult);
	return;
}

int CDBConnectorTLS::CConnector::GetLastError(void)
{
	unsigned int errNo = mysql_errno(this->_pConnector);
	return errNo;
}

CDBConnectorTLS::CDBConnectorTLS(char *szConnectIP, char *szConnectUser, char *szConnectPass, char *szConnectDBName, int iConnectPort)
{
	_iTLSIndex = TlsAlloc();

	if (TLS_OUT_OF_INDEXES == _iTLSIndex)
		CCrashDump::Crash();

	strcpy_s(this->_szConnectIP, 16, szConnectIP);
	strcpy_s(this->_szConnectUser, 32, szConnectUser);
	strcpy_s(this->_szConnectPass, 32, szConnectPass);
	strcpy_s(this->_szConnectDBName, 32, szConnectDBName);
	this->_iConnectPort = iConnectPort;
}

CDBConnectorTLS::~CDBConnectorTLS()
{
	CConnector *pConn = NULL;

	while (this->_connectorStack.GetUseSize())
	{
		this->_connectorStack.Pop(&pConn);
		delete pConn;
	}

	TlsFree(this->_iTLSIndex);
}

bool CDBConnectorTLS::Query(char *szQuery, ...)
{
	CConnector *pConn = this->GetConnector();

	char query[1024] = { 0, };
	va_list vl;

	va_start(vl, szQuery);
	vsprintf_s(query, 1024, szQuery, vl);
	va_end(vl);

	bool bResult = false;
	if (pConn->Query(query))
		bResult = true;
	
	wchar_t saveQuery[1024] = { 0, };
	mbstowcs_s(NULL, saveQuery, 1024, query, 1024);
	SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L" %s", saveQuery);
	return bResult;
}

MYSQL_ROW CDBConnectorTLS::FetchRow(void)
{
	CConnector *pConn = this->GetConnector();
	return pConn->FetchRow();
}

void CDBConnectorTLS::FreeResult(void)
{
	CConnector *pConn = this->GetConnector();
	pConn->FreeResult();
	return;
}

int CDBConnectorTLS::GetLastError(void)
{
	CConnector *pConn = this->GetConnector();
	return pConn->GetLastError();
}