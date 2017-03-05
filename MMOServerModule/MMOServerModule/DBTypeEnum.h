#pragma once

#define dfID_MAX_LEN 20
#define dfNICK_MAX_LEN 20

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

	enDB_ACCOUNT_WRITE_STATUS_LOGOUT				// 플레이어 로그아웃시 status 를 로그아웃 상태로 변경
};


//--------------------------------------------------------
// 각 타입별 사용 구조체.
//
// 경우에 따라서 구조체가 없기도 함.  
// 그런경우는 주석으로 내용을 적어두도록 한다.
//
// enum 과 똑같은 이름으로 만들며 앞 st 로 구분하며, 뒤에 in, out 을 붙인다.
//--------------------------------------------------------


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
	__int64		AccountNo;
};


struct stDB_ACCOUNT_READ_USER_out
{
	WCHAR		szID[dfID_MAX_LEN];
	WCHAR		szNick[dfNICK_MAX_LEN];
	int			Status;
	int			Party;					// 파티 1 / 2
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

