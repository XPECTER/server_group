#pragma once

//--------------------------------------------------------
// DB 의 정보를 얻기,저장하기 타입.
//
// Read, Write 통합으로 감.  
// 이를 별도로 할 경우 실수로 같은 값을 사용해도 확인하기 어려움이 있을 수 있음.
//--------------------------------------------------------
enum en_DB_ACTION_TYPE
{
	enDB_ACCOUNT_READ_LOGIN_SESSION	= 0,			// accountdb 에서 로그인 세션키 및 정보 확인 (로그인서버에서 사용)
	enDB_ACCOUNT_READ_USER,							// accountdb 에서 회원 정보 얻기 & status login 상태로

	enDB_GAME_READ_PLAYER_CHECK,					// gamedb 에서 플레이어 정보가 있는지 확인, 없다면 생성. 
													// 얻을 내용은 없음. 

	enDB_GAME_WRITE_LOG_JOIN,						// 캐릭터 선택 후 게임진입시
													// gameDB 에 저장할 내용은 없으나 로그를 남기기 위함

	enDB_GAME_WRITE_LOG_LEAVE,						// 게임모드에서 게임서버를 나갈 때 저장.
													// gameDB 에 저장할 내용은 없으나 로그를 남기기 위함

	enDB_GAME_WRITE_PLAYER_DIE,					// 플레이어 돌아가심 / die 카운트 + 1
	enDB_GAME_WRITE_PLAYER_KILL,					// 플레이어 적군 킬 / kill 카운트 + 1
													// 대상이 게스트 계정이라면 게스트 킬로 저장
													// 대상이나 자신이 더미라면 일단 저장으로 진행 후
													// DB 부하가 너무 심하다면 빼서 비교 해 봄.

	enDB_ACCOUNT_WRITE_STATUS_LOGOUT,				// 플레이어 로그아웃시 status 를 로그아웃 상태로 변경
	enDB_ACCOUNT_READ_STATUS_INIT,
};





// enDB_ACCOUNT_READ_LOGIN_SESSION ----------------------------------------------------------

struct stDB_ACCOUNT_READ_LOGIN_SESSION_in
{
	__int64		AccountNo;
	char		SessionKey[64];
};


struct stDB_ACCOUNT_READ_LOGIN_SESSION_out
{
	WCHAR		szID[dfID_MAX_LEN];
	WCHAR		szNick[dfNICK_MAX_LEN];
	int			Status;
};



// enDB_ACCOUNT_READ_USER ----------------------------------------------------------

struct stDB_ACCOUNT_READ_USER_in
{
	__int64				AccountNo;

	//CLIENT_CONNECT_INFO		ConnectInfo;
};


struct stDB_ACCOUNT_READ_USER_out
{
	WCHAR		szID[dfID_MAX_LEN];
	WCHAR		szNick[dfNICK_MAX_LEN];
	int			Status;
	int			Party;					// 파티 1 / 2
};






// enDB_GAME_READ_PLAYER_CHECK ----------------------------------------------------------

struct stDB_GAME_READ_PLAYER_CHECK_in
{
	__int64					AccountNo;
	//CLIENT_CONNECT_INFO		ConnectInfo;
};



// enDB_GAME_WRITE_LOG_JOIN --------------------------------------------------------------

struct stDB_GAME_WRITE_LOG_JOIN_in
{
	__int64		AccountNo;

	int			TileX;
	int			TileY;

	BYTE		CharacterType;
};

// enDB_GAME_WRITE_LOG_LEAVE --------------------------------------------------------------

struct stDB_GAME_WRITE_LOG_LEAVE_in
{
	__int64		AccountNo;

	int			TileX;
	int			TileY;

	int			KillCount;
	int			GuestKillCount;
};


// enDB_GAME_WRITE_PLAYER_DIE ----------------------------------------------------------

struct stDB_GAME_WRITE_PLAYER_DIE_in
{
	__int64		AccountNo;

	__int64		AttackerAccountNo;		// gameDB 에 저장하진 않으나 Log 에 남기기 위함

	int			DiePosX;				// gameDB 에는 필요가 없으나 Log 에 죽은위치 표시를 위함.
	int			DiePosY;		
};



// enDB_GAME_WRITE_PLAYER_KILL ----------------------------------------------------------

struct stDB_GAME_WRITE_PLAYER_KILL_in
{
	__int64		AccountNo;

	__int64		TargetAccountNo;		// gameDB 에 저장하진 않으나 Log 에 남기기 위함

	int			KillPosX;				// gameDB 에는 필요가 없으나 Log 에 죽은위치 표시를 위함.
	int			KillPosY;		
};


// enDB_ACCOUNT_WRITE_STATUS_LOGOUT ----------------------------------------------------------

struct stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in
{
	__int64		AccountNo;
};







//--------------------------------------------------------
// DB 저장 스레드용 메시지
//
//--------------------------------------------------------
#define dfDBWRITER_MSG_MAX	200

#define dfDBWRITER_TYPE_ACCOUNT		1		// AccountDB 
#define dfDBWRITER_TYPE_GAME		2		// GameDB
#define dfDBWRITER_TYPE_HEARTBEAT	3		// DBThread Heartbeat

//--------------------------------------------------------
// DB 저장 메시지 통합본.
//--------------------------------------------------------
typedef struct _st_DBWRITER_MSG
{
	int					Type_DB;			// AccountDB, GameDB 구분	
	en_DB_ACTION_TYPE	Type_Message;		// Message 구분

	BYTE	Message[dfDBWRITER_MSG_MAX];	// 메시지 역할을 할 데이터 영역
											// 아래의 구조체들을 모두 포함 할 수 있는 사이즈.
											// 공용으로 사용한다.
} st_DBWRITER_MSG;






// 모든 DB 저장 관련 쿼리는 DBWrite 에서 전담해야 한다.
//
// DB 저장 메시지는 테이블 단위가 아닌 컨텐츠 단위로 가는것이 맞다.
// 하나의 메시지 처리를 위한 내용을 트랜젝션 걸어서 처리 해야 함.
// 그러므로 하나의 st_DBWRITER_MSG 메시지에 하나의 쿼리만 해야 하는것이 아님.

