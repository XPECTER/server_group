#include "stdafx.h"
#include "DBConnector.h"

CDBConnector::CDBConnector()
{
	mysql_init(&this->auth_accountdb);
	mysql_init(&this->auth_gamedb);
	mysql_init(&this->auth_logdb);

	mysql_init(&this->game_accountdb);
	mysql_init(&this->game_gamedb);
	mysql_init(&this->game_logdb);

	mysql_init(&this->db_accountdb);
	mysql_init(&this->db_gamedb);
	mysql_init(&this->db_logdb);
}

CDBConnector::~CDBConnector()
{

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

	mysql_real_connect(&this->auth_accountdb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
	mysql_real_connect(&this->auth_gamedb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
	mysql_real_connect(&this->auth_logdb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);

	mysql_real_connect(&this->game_accountdb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
	mysql_real_connect(&this->game_gamedb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
	mysql_real_connect(&this->game_logdb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);

	mysql_real_connect(&this->db_accountdb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
	mysql_real_connect(&this->db_gamedb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
	mysql_real_connect(&this->db_logdb, szConvertIP, szConvertUser, szConvertPassword, szConvertDBName, iPort, (char*)NULL, 0);
}