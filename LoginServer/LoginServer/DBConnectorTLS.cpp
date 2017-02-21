#include "stdafx.h"
#include "DBConnect.h"
#include "DBConnectorTLS.h"
#include "LoginServer.h"
#include "main.h"

CDBConnectorTLS::CDBConnectorTLS()
{
	this->_iTlsIndex = TlsAlloc();
	if (TLS_OUT_OF_INDEXES == this->_iTlsIndex)
		CCrashDump::Crash();
}

CDBConnectorTLS::~CDBConnectorTLS()
{
	CDBConnector *pConn;
	while (this->_stack.Pop(&pConn))
	{
		delete pConn;
	}

	TlsFree(this->_iTlsIndex);
}

CDBConnector* CDBConnectorTLS::GetInstance(void)
{
	CDBConnector *pDB = (CDBConnector *)TlsGetValue(this->_iTlsIndex);

	if (nullptr == pDB)
	{
		pDB = new CDBConnector;
		this->_stack.Push(pDB);

		if (!pDB->Connect(g_ConfigData._szAccountIP, g_ConfigData._szAccountUser, g_ConfigData._szAccountPassword, g_ConfigData._szAccountDBName, g_ConfigData._iAccountPort))
		{
			SYSLOG(L"DATABASE", LOG::LEVEL_DEBUG, L" ## DATABASE ErrorNo : %d", this->GetLastError());
		}

		TlsSetValue(this->_iTlsIndex, pDB);
	}
	
	return pDB;
}

bool CDBConnectorTLS::Connect()
{
	CDBConnector *pDB = this->GetInstance();
	if (nullptr == pDB)
		return false;
	else
		return true;
}

bool CDBConnectorTLS::Query(char *szQuery, ...)
{
	CDBConnector *pDB = this->GetInstance();

	char query[1024] = { 0, };
	va_list vl;
	
	va_start(vl, szQuery);
	vsprintf_s(query, 1024, szQuery, vl);
	va_end(vl);

	return pDB->Query(query);
}

MYSQL_ROW CDBConnectorTLS::FetchRow(void)
{
	CDBConnector *pDB = this->GetInstance();
	return pDB->FetchRow();
}

void CDBConnectorTLS::FreeResult(void)
{
	CDBConnector *pDB = this->GetInstance();
	pDB->FreeResult();
	return;
}

int CDBConnectorTLS::GetLastError(void)
{
	CDBConnector *pDB = this->GetInstance();
	return pDB->GetLastError();
}

bool AccountDB::ReadDB(en_DB_ACTION_TYPE type, void *pIn, void *pOut)
{
	MYSQL_ROW row;
	

	switch (type)
	{
		case enDB_ACCOUNT_READ_LOGIN_SESSION:
		{
			stDB_ACCOUNT_READ_LOGIN_SESSION_in *pInput = (stDB_ACCOUNT_READ_LOGIN_SESSION_in *)pIn;
			stDB_ACCOUNT_READ_LOGIN_SESSION_out *pOutput = (stDB_ACCOUNT_READ_LOGIN_SESSION_out *)pOut;
			__int64 accountno = pInput->AccountNo;

			pOutput->Status = dfLOGIN_STATUS_OK;

			if (this->Query("SELECT accountno, userid, usernick FROM `accountdb`.`account` WHERE accountno = %d;", accountno))
			{
				row = this->FetchRow();
				if (NULL == row[0])
				{
					pOutput->Status = dfLOGIN_STATUS_ACCOUNT_MISS;
				}
				else
				{
					int len = MultiByteToWideChar(CP_UTF8, 0, row[1], -1, NULL, NULL);
					MultiByteToWideChar(CP_UTF8, 0, row[1], -1, pOutput->szID, len);

					len = MultiByteToWideChar(CP_UTF8, 0, row[2], -1, NULL, NULL);
					MultiByteToWideChar(CP_UTF8, 0, row[2], -1, pOutput->szNick, len);
				}

				this->FreeResult();
			}
			else
			{
				SYSLOG(L"DATABASE", LOG::LEVEL_DEBUG, L" ##DATABASE Query : %s, ErrorNo : %d", this->_szLastQuery, this->GetLastError());
				CCrashDump::Crash();
			}

			if (this->Query("SELECT accountno, sessionkey FROM `accountdb`.`sessionkey` WHERE accountno = %d;", accountno))
			{
				row = this->FetchRow();
				if (NULL == row[0])
				{
					pOutput->Status = dfLOGIN_STATUS_SESSION_MISS;
				}
				else
				{
					if (dfDUMMY_ACCOUNTNO_MAX < pInput->AccountNo)
					{
						if (0 != memcmp(pInput->SessionKey, row[0], 64))
							pOutput->Status = dfLOGIN_STATUS_FAIL;
					}
				}

				this->FreeResult();
			}
			else
			{
				SYSLOG(L"DATABASE", LOG::LEVEL_DEBUG, L" ##DATABASE Query : %s, ErrorNo : %d", this->_szLastQuery, this->GetLastError());
				CCrashDump::Crash();
			}

			if (this->Query("SELECT accountno, status FROM `accountdb`.`status` WHERE accountno = %d;", accountno))
			{
				row = this->FetchRow();
				if (NULL == row[0])
				{
					pOutput->Status = dfLOGIN_STATUS_STATUS_MISS;
				}
				else
				{
					if (0 == strcmp("1", row[1]))
					{
						pOutput->Status = dfLOGIN_STATUS_GAME;
					}
				}

				this->FreeResult();
			}
			else
			{
				SYSLOG(L"DATABASE", LOG::LEVEL_DEBUG, L" ##DATABASE Query : %s, ErrorNo : %d", this->_szLastQuery, this->GetLastError());
				CCrashDump::Crash();
			}

			break;
		}

		case enDB_ACCOUNT_READ_WHITE_IP:
			break;

		case enDB_ACCOUNT_READ_RESET_STATUS_ALL:
			this->Query("UPDATE `accountdb`.`status` SET status = 0;");
			break;
	}

	return true;
}

bool AccountDB::WriteDB(en_DB_ACTION_TYPE type, void *pIn)
{
	return true;
}