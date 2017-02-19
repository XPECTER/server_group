#pragma once

//--------------------------------------------------------
// DB �� ������ ���,�����ϱ� Ÿ��.
//
// Read, Write �������� ��.  
// �̸� ������ �� ��� �Ǽ��� ���� ���� ����ص� Ȯ���ϱ� ������� ���� �� ����.
//--------------------------------------------------------
enum en_DB_ACTION_TYPE
{
	enDB_ACCOUNT_READ_LOGIN_SESSION	= 0,
	enDB_ACCOUNT_READ_WHITE_IP,
	
	enDB_ACCOUNT_READ_RESET_STATUS_ALL,					// �α��� ������ ó�� ������, ���� ������ �ٲ𶧴� ��� ����ڰ� �α׾ƿ� �Ȱɷ� �����.

};





//--------------------------------------------------------
// �� Ÿ�Ժ� ��� ����ü.
//
// ��쿡 ���� ����ü�� ���⵵ ��.  
// �׷����� �ּ����� ������ ����ε��� �Ѵ�.
//
// enum �� �Ȱ��� �̸����� ����� �� st �� �����ϸ�, �ڿ� in, out �� ���δ�.
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




