#pragma once

//--------------------------------------------------------
// DB 의 정보를 얻기,저장하기 타입.
//
// Read, Write 통합으로 감.  
// 이를 별도로 할 경우 실수로 같은 값을 사용해도 확인하기 어려움이 있을 수 있음.
//--------------------------------------------------------
enum en_DB_ACTION_TYPE
{
	enDB_ACCOUNT_READ_LOGIN_SESSION	= 0,
	enDB_ACCOUNT_READ_WHITE_IP,
	
	enDB_ACCOUNT_READ_RESET_STATUS_ALL,					// 로그인 서버가 처음 켜질때, 서비스 서버가 바뀔때는 모든 사용자가 로그아웃 된걸로 만든다.

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
	WCHAR		szID[20];
	WCHAR		szNick[20];
	int			Status;
};




