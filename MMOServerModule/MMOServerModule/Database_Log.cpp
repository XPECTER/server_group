#include "stdafx.h"
#include "GameServer.h"
#include "DBTypeEnum.h"
#include "Database_Log.h"

LogDB::LogDB(char *szConnectIP, char *szConnectUser, char *szConnectPass, char *szConnectDBName, int iConnectPort, CGameServer *pGameServer)
	: CDBConnectorTLS(szConnectIP, szConnectUser, szConnectPass, szConnectDBName, iConnectPort)
{
	this->_pGameServer = pGameServer;
}

LogDB::~LogDB()
{

}

bool LogDB::QueryDB(WORD type, WORD code, __int64 AccountNo, wchar_t *szServerName, __int64 param1, __int64 param2, __int64 param3, __int64 param4, wchar_t *szMessage)
{
	if (!this->Query("INSERT INTO `logdb`.`gamelog` VALUES (%d, %d, %d, %s, %d, %d, %d, %d, %s)", 
		type, code, AccountNo, szServerName, param1, param2, param3, param4, szMessage))
	{
		SYSLOG(L"DBLOG", LOG::LEVEL_ERROR, L"%d, %d, %d, %s, %d, %d, %d, %d, %s",
			type, code, AccountNo, szServerName, param1, param2, param3, param4, szMessage);
		return false;
	}

	return true;
}