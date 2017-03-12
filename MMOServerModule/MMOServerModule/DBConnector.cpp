#include "stdafx.h"
#include "DBConnector.h"

//CDBConnector::CDBConnector()
//{
//	mysql_init(&this->auth_accountdb);
//	mysql_init(&this->auth_gamedb);
//	mysql_init(&this->auth_logdb);
//
//	mysql_init(&this->game_accountdb);
//	mysql_init(&this->game_gamedb);
//	mysql_init(&this->game_logdb);
//
//	mysql_init(&this->db_accountdb);
//	mysql_init(&this->db_gamedb);
//	mysql_init(&this->db_logdb);
//}
//
//CDBConnector::~CDBConnector()
//{
//
//}
//
//bool CDBConnector::Connect(wchar_t *szConnectIP, wchar_t *szUser, wchar_t *szPassword, wchar_t *szDBName, int iPort)
//{
//	size_t i;
//	char szConvertIP[16] = { 0, };
//	char szConvertUser[32] = { 0, };
//	char szConvertPassword[32] = { 0, };
//	char szConvertDBName[32] = { 0, };
//
//	wcstombs_s(&i, szConvertIP, 16, szConnectIP, 16);
//	wcstombs_s(&i, szConvertUser, 32, szUser, 32);
//	wcstombs_s(&i, szConvertPassword, szPassword, 32);
//	wcstombs_s(&i, szConvertDBName, szDBName, 32);
//
//	mysql_real_connect(&this->auth_accountdb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
//	mysql_real_connect(&this->auth_gamedb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
//	mysql_real_connect(&this->auth_logdb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
//
//	mysql_real_connect(&this->game_accountdb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
//	mysql_real_connect(&this->game_gamedb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
//	mysql_real_connect(&this->game_logdb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
//
//	mysql_real_connect(&this->db_accountdb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
//	mysql_real_connect(&this->db_gamedb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
//	mysql_real_connect(&this->db_logdb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
//}


CDBConnectorTLS::CConnector::CConnector()
{
	mysql_init(&this->_connection_AccountDB);
	mysql_init(&this->_connection_GameDB);
	mysql_init(&this->_connection_LogDB);

	this->_dbType = 0;
}

CDBConnectorTLS::CConnector::~CConnector()
{
	mysql_close(&this->_connection_AccountDB);
	mysql_close(&this->_connection_GameDB);
	mysql_close(&this->_connection_LogDB);
}

void CDBConnectorTLS::CConnector::ConnectDB(void)
{
	_pConnection_AccountDB = mysql_real_connect(&this->_connection_AccountDB, g_Config.szAccountDBIP, g_Config.szAccountDBUser, g_Config.szAccountDBPassword, g_Config.szAccountDBName, g_Config.iAccountDBPort, (char *)NULL, 0);
	if (NULL == _pConnection_AccountDB)
		CCrashDump::Crash();

	// 커넥트 2번하면 안되네?
	_pConnection_GameDB = mysql_real_connect(&this->_connection_GameDB, g_Config.szGameDBIP, g_Config.szGameDBUser, g_Config.szGameDBPassword, g_Config.szGameDBName, g_Config.iGameDBPort, (char *)NULL, 0);
	if (NULL == _pConnection_GameDB)
		CCrashDump::Crash();

	/*_pConnection_LogDB = mysql_real_connect(&this->_connection_LogDB, g_Config.szLogDBIP, g_Config.szLogDBUser, g_Config.szLogDBPassword, g_Config.szLogDBName, g_Config.iLogDBPort, (char *)NULL, 0);
	if (NULL == _pConnection_LogDB)
		CCrashDump::Crash();*/

	return;
}

CDBConnectorTLS::CDBConnectorTLS()
{
	_iTLSIndex = TlsAlloc();

	if (TLS_OUT_OF_INDEXES == _iTLSIndex)
		CCrashDump::Crash();
}

CDBConnectorTLS::~CDBConnectorTLS()
{
	CConnector *pConn = NULL;

	while (this->_connectorQueue.GetUseSize())
	{
		this->_connectorQueue.Dequeue(&pConn);
		delete pConn;
	}

	TlsFree(this->_iTLSIndex);
}

CDBConnectorTLS::CConnector* CDBConnectorTLS::GetConnector(void)
{
	CConnector *pConn = (CConnector *)TlsGetValue(this->_iTLSIndex);

	if (NULL == pConn)
	{
		pConn = new CConnector;
		pConn->ConnectDB();

		TlsSetValue(this->_iTLSIndex, (LPVOID)pConn);
		this->_connectorQueue.Enqueue(pConn);
	}

	return pConn;
}

bool CDBConnectorTLS::CConnector::Query(int iDBType, char *szQuery)
{
	this->_dbType = iDBType;
	bool bResult = false;

	switch (iDBType)
	{
		case en_DB_TYPE_ACCOUNT:
		{
			if (0 == mysql_query(this->_pConnection_AccountDB, szQuery))
			{
				this->_pResult_AccountDB = mysql_store_result(&this->_connection_AccountDB);
				bResult = true;
			}
			break;
		}

		case en_DB_TYPE_GAME:
		{
			if (0 == mysql_query(this->_pConnection_GameDB, szQuery))
			{
				this->_pResult_GameDB = mysql_store_result(&this->_connection_GameDB);
				bResult = true;
			}
			break;
		}

		case en_DB_TYPE_LOG:
		{
			if (0 == mysql_query(this->_pConnection_LogDB, szQuery))
			{
				this->_pResult_LogDB = mysql_store_result(&this->_connection_LogDB);
				bResult = true;
			}
			break;
		}
	}

	return bResult;
}

MYSQL_ROW CDBConnectorTLS::CConnector::FetchRow(void)
{
	MYSQL_ROW row = NULL;

	switch (this->_dbType)
	{
		case en_DB_TYPE_ACCOUNT:
		{
			row = mysql_fetch_row(this->_pResult_AccountDB);
			break;
		}

		case en_DB_TYPE_GAME:
		{
			row = mysql_fetch_row(this->_pResult_GameDB);
			break;
		}

		/*case en_DB_TYPE_LOG:
		{
			row = mysql_fetch_row(this->_pResult_LogDB);
			break;
		}*/

		default:
		{
			SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"FetchRow Error");
			CCrashDump::Crash();
		}
	}

	return row;
}

void CDBConnectorTLS::CConnector::FreeResult(void)
{
	switch (this->_dbType)
	{
		case en_DB_TYPE_ACCOUNT:
		{
			mysql_free_result(this->_pResult_AccountDB);
			return;
		}

		case en_DB_TYPE_GAME:
		{
			mysql_free_result(this->_pResult_GameDB);
			return;
		}

		/*case en_DB_TYPE_LOG:
		{
			 mysql_free_result(this->_pResult_LogDB);
			 return;
		}*/

		default:
		{
			SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"FreeResult Error");
			CCrashDump::Crash();
		}
	}

	return;
}

int CDBConnectorTLS::CConnector::GetLastError(void)
{
	unsigned int errNo = 0;

	switch (this->_dbType)
	{
		case en_DB_TYPE_ACCOUNT:
		{
			errNo = mysql_errno(&this->_connection_AccountDB);
		}

		case en_DB_TYPE_GAME:
		{
			errNo = mysql_errno(&this->_connection_GameDB);
		}

		case en_DB_TYPE_LOG:
		{
			errNo = mysql_errno(&this->_connection_LogDB);
		}

		default:
		{
			SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"FreeResult Error");
			CCrashDump::Crash();
		}
	}

	return errNo;
}

bool CDBConnectorTLS::Query(int iDBType, char *szQuery, ...)
{
	CConnector *pConn = this->GetConnector();

	char query[1024] = { 0, };
	va_list vl;

	va_start(vl, szQuery);
	vsprintf_s(query, 1024, szQuery, vl);
	va_end(vl);

	pConn->Query(iDBType, query);

	wchar_t saveQuery[1024] = { 0, };
	mbstowcs_s(NULL, saveQuery, 1024, query, 1024);
	SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L" %s", saveQuery);
	return true;
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

void CDatabase::Query_AccountDB(en_DB_ACTION_TYPE type, PVOID pIn, PVOID pOut)
{
	MYSQL_ROW row;

	switch (type)
	{
		/*case enDB_ACCOUNT_READ_LOGIN_SESSION:
		{
			stDB_ACCOUNT_READ_LOGIN_SESSION_in *input = (stDB_ACCOUNT_READ_LOGIN_SESSION_in *)pIn;
			stDB_ACCOUNT_READ_LOGIN_SESSION_out *output = (stDB_ACCOUNT_READ_LOGIN_SESSION_out *)pOut;

			this->Query(en_DB_TYPE_ACCOUNT, "");
			break;
		}*/

		case enDB_ACCOUNT_READ_USER:
		{
			stDB_ACCOUNT_READ_USER_in *input = (stDB_ACCOUNT_READ_USER_in *)pIn;
			stDB_ACCOUNT_READ_USER_out * output = (stDB_ACCOUNT_READ_USER_out *)pOut;
			__int64 accountno = input->AccountNo;

#pragma region checkStatus
			this->Query(en_DB_TYPE_ACCOUNT, "SELECT accountno, status FROM `accountdb`.`status` WHERE accountno = %d", accountno);
			row = this->FetchRow();

			if (NULL == row[0])
			{
				output->Status = dfGAME_LOGIN_FAIL;
			}
			else
			{
				if (0 == strcmp(row[1], "1"))
				{
					output->Status = dfGAME_LOGIN_FAIL;
				}
			}

			this->FreeResult();
#pragma endregion checkStatus

#pragma region selectUserinfo
			this->Query(en_DB_TYPE_ACCOUNT, "SELECT accountno, userid, usernick, gamecodi_party FROM `accountdb`.`account` WHERE accountno = %d", accountno);
			row = this->FetchRow();

			if (NULL == row[0])
			{
				output->Status = dfGAME_LOGIN_FAIL;
			}
			else
			{
				int len = MultiByteToWideChar(CP_UTF8, 0, row[1], -1, NULL, NULL);
				MultiByteToWideChar(CP_UTF8, 0, row[1], -1, output->szID, len);

				len = MultiByteToWideChar(CP_UTF8, 0, row[2], -1, NULL, NULL);
				MultiByteToWideChar(CP_UTF8, 0, row[2], -1, output->szNick, len);

				output->Status = atoi(row[4]);
				output->Party = atoi(row[4]);
			}

			this->FreeResult();
#pragma endregion selectUserinfo
			
			// Update status 1
			this->Query(en_DB_TYPE_ACCOUNT, "UPDATE `accountdb`.`status` SET status = 1 WHERE accountno = %d", accountno);
			
			break;
		}

		case enDB_ACCOUNT_WRITE_STATUS_LOGOUT:
		{
			stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in *input = (stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in *)pIn;
			this->Query(en_DB_TYPE_ACCOUNT, "UPDATE `accountdb`.`status` SET status = 0 WHERE accountno = %d", input->AccountNo);
			break;
		}

		default:
		{
			SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"Wrong DB_ACTION_TYPE");
			CCrashDump::Crash();
		}
	}

	return;
}

void CDatabase::Query_GameDB(en_DB_ACTION_TYPE type, PVOID pIn, PVOID pOut)
{
	return;
}

CDatabase::CDatabase()
{

}

CDatabase::~CDatabase()
{

}