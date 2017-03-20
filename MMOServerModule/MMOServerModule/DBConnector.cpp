#include "stdafx.h"
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

//void CDatabase::Query_AccountDB(en_DB_ACTION_TYPE type, PVOID pIn, PVOID pOut)
//{
//	MYSQL_ROW row;
//
//	switch (type)
//	{
//		/*case enDB_ACCOUNT_READ_LOGIN_SESSION:
//		{
//			stDB_ACCOUNT_READ_LOGIN_SESSION_in *input = (stDB_ACCOUNT_READ_LOGIN_SESSION_in *)pIn;
//			stDB_ACCOUNT_READ_LOGIN_SESSION_out *output = (stDB_ACCOUNT_READ_LOGIN_SESSION_out *)pOut;
//
//			this->Query(en_DB_TYPE_ACCOUNT, "");
//			break;
//		}*/
//
//		case enDB_ACCOUNT_READ_USER:
//		{
//			stDB_ACCOUNT_READ_USER_in *input = (stDB_ACCOUNT_READ_USER_in *)pIn;
//			stDB_ACCOUNT_READ_USER_out * output = (stDB_ACCOUNT_READ_USER_out *)pOut;
//			__int64 accountno = input->AccountNo;
//
//#pragma region checkStatus
//			this->Query(en_DB_TYPE_ACCOUNT, "SELECT accountno, status FROM `accountdb`.`status` WHERE accountno = %d", accountno);
//			row = this->FetchRow();
//
//			if (NULL == row[0])
//			{
//				output->Status = dfGAME_LOGIN_FAIL;
//			}
//			else
//			{
//				if (0 == strcmp(row[1], "1"))
//				{
//					output->Status = dfGAME_LOGIN_FAIL;
//				}
//			}
//
//			this->FreeResult();
//#pragma endregion checkStatus
//
//#pragma region selectUserinfo
//			this->Query(en_DB_TYPE_ACCOUNT, "SELECT accountno, userid, usernick, gamecodi_party FROM `accountdb`.`account` WHERE accountno = %d", accountno);
//			row = this->FetchRow();
//
//			if (NULL == row[0])
//			{
//				output->Status = dfGAME_LOGIN_FAIL;
//			}
//			else
//			{
//				int len = MultiByteToWideChar(CP_UTF8, 0, row[1], -1, NULL, NULL);
//				MultiByteToWideChar(CP_UTF8, 0, row[1], -1, output->szID, len);
//
//				len = MultiByteToWideChar(CP_UTF8, 0, row[2], -1, NULL, NULL);
//				MultiByteToWideChar(CP_UTF8, 0, row[2], -1, output->szNick, len);
//
//				output->Status = atoi(row[4]);
//				output->Party = atoi(row[4]);
//			}
//
//			this->FreeResult();
//#pragma endregion selectUserinfo
//			
//			// Update status 1
//			this->Query(en_DB_TYPE_ACCOUNT, "UPDATE `accountdb`.`status` SET status = 1 WHERE accountno = %d", accountno);
//			
//			break;
//		}
//
//		case enDB_ACCOUNT_WRITE_STATUS_LOGOUT:
//		{
//			stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in *input = (stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in *)pIn;
//			this->Query(en_DB_TYPE_ACCOUNT, "UPDATE `accountdb`.`status` SET status = 0 WHERE accountno = %d", input->AccountNo);
//			break;
//		}
//
//		default:
//		{
//			SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"Wrong DB_ACTION_TYPE");
//			CCrashDump::Crash();
//		}
//	}
//
//	return;
//}

//void CDatabase::Query_GameDB(en_DB_ACTION_TYPE type, PVOID pIn, PVOID pOut)
//{
//	return;
//}


AccountDB::AccountDB(char *szConnectIP, char *szConnectUser, char *szConnectPass, char *szConnectDBName, int iConnectPort) 
	: CDBConnectorTLS(szConnectIP, szConnectUser, szConnectPass, szConnectDBName, iConnectPort)
{

}

AccountDB::~AccountDB()
{

}

void AccountDB::QueryDB(en_DB_ACTION_TYPE type, PVOID pIn, PVOID pOut)
{
	MYSQL_ROW row;

	switch (type)
	{
		/////////////////////////////////////////////////////////////////////
		//
		//	enDB_ACCOUNT_READ_USER
		//	
		//	게임서버로 로그인할 때 필요한 유저 정보를 가져온다.
		//
		/////////////////////////////////////////////////////////////////////
		case enDB_ACCOUNT_READ_USER:
		{
			stDB_ACCOUNT_READ_USER_in *in = (stDB_ACCOUNT_READ_USER_in *)pIn;
			stDB_ACCOUNT_READ_USER_out *out = (stDB_ACCOUNT_READ_USER_out *)pOut;
			__int64 accountNo = in->AccountNo;

			// 현재 게임중인지 검사
			if (this->Query("SELECT accountno, status FROM `accountdb`.`status` WHERE accountno = %d", accountNo))
			{
				row = this->FetchRow();

				if (NULL == row[0])
				{
					// Row가 없음
					out->Status = dfGAME_LOGIN_FAIL;
					break;
				}
				else
				{
					if (0 == strcmp(row[1], "1"))
					{
						// 아직 status가 0으로 바뀌지 않음. 또는 게임중
						out->Status = dfGAME_LOGIN_FAIL;
						break;
					}
				}

				this->FreeResult();
			}
			else
			{
				SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"Query Failed. Errno : %d", this->GetLastError());
				CCrashDump::Crash();
			}
		
			// 유저 정보 세팅
			if (this->Query("SELECT accountno, userid, usernick, gamecodi_party FROM `accountdb`.`account` WHERE accountno = %d", accountNo))
			{
				row = this->FetchRow();
				
				if (NULL == row[0])
				{
					// Row가 없음
					out->Status = dfGAME_LOGIN_FAIL;
					break;
				}
				else
				{
					// 정보 세팅
					// ID
					int len = MultiByteToWideChar(CP_UTF8, 0, row[1], -1, NULL, NULL);
					MultiByteToWideChar(CP_UTF8, 0, row[1], -1, out->szID, len);

					// Nick
					len = MultiByteToWideChar(CP_UTF8, 0, row[2], -1, NULL, NULL);
					MultiByteToWideChar(CP_UTF8, 0, row[2], -1, out->szNick, len);

					// Status
					out->Status = atoi(row[3]);

					// Party
					out->Party = atoi(row[3]);
				}

				this->FreeResult();
			}
			else
			{
				SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"Query Failed. Errno : %d", this->GetLastError());
				CCrashDump::Crash();
			}

			// 로그인 완료로 처리하고 게임상태로 바꿈
			if (!this->Query("UPDATE `accountdb`.`status` SET status = 1 WHERE accountno = %d", accountNo))
			{
				SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"Query Failed. Errno : %d", this->GetLastError());
				CCrashDump::Crash();
			}

			break;
		}

		/////////////////////////////////////////////////////////////////////
		//
		//	enDB_ACCOUNT_READ_STATUS_INIT
		//	
		//	게임 서버를 켤 때 `accountdb`.`status`의 status 항목을 모두 0으로
		//	초기화 한다.
		//
		/////////////////////////////////////////////////////////////////////
		case enDB_ACCOUNT_READ_STATUS_INIT:
		{
			if (!this->Query("UPDATE `accountdb`.`status` SET status = 0"))
			{
				if (1175 == this->GetLastError())
				{
					if (!this->Query("set sql_safe_updates = 0"))
					{
						SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"Query Failed. Errno : %d", this->GetLastError());
						CCrashDump::Crash();
					}
					else
					{
						if (!this->Query("UPDATE `accountdb`.`status` SET status = 0"))
						{
							SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"Query Failed. Errno : %d", this->GetLastError());
							CCrashDump::Crash();
						}
					}
				}
				else
				{
					SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"Query Failed. Errno : %d", this->GetLastError());
					CCrashDump::Crash();
				}
			}
			break;
		}


		/////////////////////////////////////////////////////////////////////
		//
		//	enDB_ACCOUNT_WRITE_STATUS_LOGOUT
		//	
		//	유저가 게임에서 나갈 때 `accountdb`.`status`의 status 항목을
		//	0으로 바꾼다.
		//
		/////////////////////////////////////////////////////////////////////
		case enDB_ACCOUNT_WRITE_STATUS_LOGOUT:
		{
			stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in *in = (stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in *)pIn;
			if (!this->Query("UPDATE `accountdb`.`status` SET status = 0 WHERE accountno = %d", in->AccountNo))
			{
				SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"Query Failed");
				CCrashDump::Crash();
			}
			break;
		}
	}
}

GameDB::GameDB(char *szConnectIP, char *szConnectUser, char *szConnectPass, char *szConnectDBName, int iConnectPort)
	: CDBConnectorTLS(szConnectIP, szConnectUser, szConnectPass, szConnectDBName, iConnectPort)
{

}

GameDB::~GameDB()
{

}

LogDB::LogDB(char *szConnectIP, char *szConnectUser, char *szConnectPass, char *szConnectDBName, int iConnectPort)
	: CDBConnectorTLS(szConnectIP, szConnectUser, szConnectPass, szConnectDBName, iConnectPort)
{

}

LogDB::~LogDB()
{

}