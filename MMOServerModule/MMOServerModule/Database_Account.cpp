#include "stdafx.h"
#include "GameServer.h"
#include "DBTypeEnum.h"
#include "Database_Account.h"

AccountDB::AccountDB(char *szConnectIP, char *szConnectUser, char *szConnectPass, char *szConnectDBName, int iConnectPort, CGameServer *pGameServer)
	: CDBConnectorTLS(szConnectIP, szConnectUser, szConnectPass, szConnectDBName, iConnectPort)
{
	this->_pGameServer = pGameServer;
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
		//	���Ӽ����� �α����� �� �ʿ��� ���� ������ �����´�.
		//
		/////////////////////////////////////////////////////////////////////
	case enDB_ACCOUNT_READ_USER:
	{
		stDB_ACCOUNT_READ_USER_in *in = (stDB_ACCOUNT_READ_USER_in *)pIn;
		stDB_ACCOUNT_READ_USER_out *out = (stDB_ACCOUNT_READ_USER_out *)pOut;
		__int64 accountNo = in->AccountNo;

		// ���� ���������� �˻�
		if (this->Query("SELECT accountno, status FROM `accountdb`.`status` WHERE accountno = %d", accountNo))
		{
			row = this->FetchRow();

			if (NULL == row[0])
			{
				// Row�� ����
				out->Status = dfGAME_LOGIN_FAIL;
				break;
			}
			else
			{
				if (0 == strcmp(row[1], "1"))
				{
					// ���� status�� 0���� �ٲ��� ����. �Ǵ� ������
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

		// ���� ���� ����
		if (this->Query("SELECT accountno, userid, usernick, gamecodi_party FROM `accountdb`.`account` WHERE accountno = %d", accountNo))
		{
			row = this->FetchRow();

			if (NULL == row[0])
			{
				// Row�� ����
				out->Status = dfGAME_LOGIN_FAIL;
				break;
			}
			else
			{
				// ���� ����
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

		// �α��� �Ϸ�� ó���ϰ� ���ӻ��·� �ٲ�
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
	//	���� ������ �� �� `accountdb`.`status`�� status �׸��� ��� 0����
	//	�ʱ�ȭ �Ѵ�.
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
	//	������ ���ӿ��� ���� �� `accountdb`.`status`�� status �׸���
	//	0���� �ٲ۴�.
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