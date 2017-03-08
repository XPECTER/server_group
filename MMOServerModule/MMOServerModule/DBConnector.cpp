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