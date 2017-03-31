#include "stdafx.h"
#include "GameServer.h"
#include "DBTypeEnum.h"
#include "Database_Game.h"

GameDB::GameDB(char *szConnectIP, char *szConnectUser, char *szConnectPass, char *szConnectDBName, int iConnectPort, CGameServer *pGameServer)
	: CDBConnectorTLS(szConnectIP, szConnectUser, szConnectPass, szConnectDBName, iConnectPort)
{
	this->_pGameServer = pGameServer;
}

GameDB::~GameDB()
{

}

void GameDB::QueryDB(en_DB_ACTION_TYPE type, PVOID pIn, PVOID pOut)
{
	switch (type)
	{
		/////////////////////////////////////////////////////////////////////
		//
		//	enDB_GAME_READ_PLAYER_CHECK
		//	
		//	게임서버로 gamedb.player 테이블에 유저 정보가 없다면 생성한다.
		//	메시지로 처리해도 된다. 이 테이블에 기록될 정보도 메시지 큐에서 
		//	메시지를 뽑아 처리하기 때문에 순서가 꼬일일은 없다
		//
		/////////////////////////////////////////////////////////////////////
		case enDB_GAME_READ_PLAYER_CHECK:
		{
			stDB_ACCOUNT_READ_USER_in *input = (stDB_ACCOUNT_READ_USER_in *)pIn;
			__int64 AccountNo = input->AccountNo;
			st_ACCEPT_CLIENT_INFO ConnectInfo = input->ConnectInfo;

			if(!this->Query("INSERT INTO `gamedb`.`player` ( `accountno` ) SELECT %lld FROM DUAL WHERE NOT EXISTS(SELECT `accountno` FROM  `gamedb`.`player` WHERE `accountno` = %lld); ", AccountNo))
			{
				SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"Query Failed. Errno : %d", this->GetLastError());
				CCrashDump::Crash();
			}

			// 로그 남기기
			wchar_t addr[16] = { 0, };
			InetNtop(AF_INET, &ConnectInfo._clientAddr.sin_addr, addr, 16);
			if (!this->_pGameServer->_database_Log->QueryDB(1, 11, AccountNo, g_Config.szServerGroupName, 0, 0, 0, ntohs(ConnectInfo._clientAddr.sin_port), addr))
			{
				// 디비가 끊어짐
			}

			break;
		}


		/////////////////////////////////////////////////////////////////////
		//
		//	enDB_GAME_WRITE_LOG_JOIN
		//	
		//	유저가 캐릭터 선택후 게임모드가 됐다면 로그를 남긴다.
		//
		/////////////////////////////////////////////////////////////////////
		case enDB_GAME_WRITE_LOG_JOIN:
		{
			stDB_GAME_WRITE_LOG_JOIN_in *input = (stDB_GAME_WRITE_LOG_JOIN_in *)pIn;
			this->_pGameServer->_database_Log->QueryDB(1, 12, input->AccountNo, g_Config.szServerGroupName, input->TileX, input->TileY, input->CharacterType, 0, NULL);
			break;
		}


		/////////////////////////////////////////////////////////////////////
		//
		//	enDB_GAME_WRITE_LOG_LEAVE
		//	
		//	유저가 게임서버를 나갈 때 로그를 남긴다.
		//
		/////////////////////////////////////////////////////////////////////
		case enDB_GAME_WRITE_LOG_LEAVE:
		{
			stDB_GAME_WRITE_LOG_LEAVE_in *input = (stDB_GAME_WRITE_LOG_LEAVE_in *)pIn;
			this->_pGameServer->_database_Log->QueryDB(1, 11, input->AccountNo, g_Config.szServerGroupName, input->TileX, input->TileY, input->KillCount, input->GuestKillCount, NULL);
			break;
		}
		

		/////////////////////////////////////////////////////////////////////
		//
		//	enDB_GAME_WRITE_PLAYER_DIE
		//	
		//	플레이어가 죽었으면 `gamedb`.`player`의 total_die를 하나 올린다.
		//	성공적으로 갱신했다면 로그를 남긴다.
		//
		/////////////////////////////////////////////////////////////////////
		case enDB_GAME_WRITE_PLAYER_DIE:
		{
			stDB_GAME_WRITE_PLAYER_DIE_in *input = (stDB_GAME_WRITE_PLAYER_DIE_in *)pIn;
			__int64 AccountNo = input->AccountNo;

			if (dfDUMMY_ACCOUNTNO_LIMIT < AccountNo && dfGUEST_ACCOUNTNO_OVER > AccountNo)
			{
				if (!this->Query("UPDATE `gamedb`.`player` SET total_die += 1 WHERE accountno = %lld", AccountNo))
				{
					SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"Query Failed. Errno : %d", this->GetLastError());
					CCrashDump::Crash();
				}

				this->_pGameServer->_database_Log->QueryDB(2, 21, AccountNo, g_Config.szServerGroupName, input->DiePosX, input->DiePosY, input->AttackerAccountNo, 0, NULL);
			}

			break;
		}


		/////////////////////////////////////////////////////////////////////
		//
		//	enDB_GAME_WRITE_PLAYER_KILL
		//	
		//	플레이어가 누군가 죽였다면 `gamedb`.`player`의 kill카운트를 하나 올린다.
		//	죽인 플레이어가 더미나 게스트 계정이면 guest_kill에, 
		//	실제 플레이하는 유저라면 total_kill에 더한다.
		//	성공적으로 갱신했다면 로그를 남긴다.
		//
		/////////////////////////////////////////////////////////////////////
		case enDB_GAME_WRITE_PLAYER_KILL:
		{
			stDB_GAME_WRITE_PLAYER_KILL_in *input = (stDB_GAME_WRITE_PLAYER_KILL_in *)pIn;
			__int64 AccountNo = input->AccountNo;
			__int64 TargetAccountNo = input->TargetAccountNo;

			if (dfDUMMY_ACCOUNTNO_LIMIT < AccountNo && dfGUEST_ACCOUNTNO_OVER > AccountNo)
			{
				if (dfDUMMY_ACCOUNTNO_LIMIT < TargetAccountNo && dfGUEST_ACCOUNTNO_OVER > AccountNo)
				{
					if (!this->Query("UPDATE `gamedb`.`player` SET total_kill += 1 WHERE accountno = %lld", AccountNo))
					{
						SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"Query Failed. Errno : %d", this->GetLastError());
						CCrashDump::Crash();
					}
				}
				else
				{
					if (!this->Query("UPDATE `gamedb`.`player` SET guest_kill += 1 WHERE accountno = %lld", AccountNo))
					{
						SYSLOG(L"DATABASE", LOG::LEVEL_ERROR, L"Query Failed. Errno : %d", this->GetLastError());
						CCrashDump::Crash();
					}
				}

				this->_pGameServer->_database_Log->QueryDB(2, 21, AccountNo, g_Config.szServerGroupName, input->KillPosX, input->KillPosY, input->TargetAccountNo, 0, NULL);
			}

			break;
		}
	}
}