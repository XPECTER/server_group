#pragma once

#define dfID_MAX_LEN 20
#define dfNICK_MAX_LEN 20

//--------------------------------------------------------
// DB �� ������ ���,�����ϱ� Ÿ��.
//
// Read, Write �������� ��.  
// �̸� ������ �� ��� �Ǽ��� ���� ���� ����ص� Ȯ���ϱ� ������� ���� �� ����.
//--------------------------------------------------------
enum en_DB_ACTION_TYPE
{
	enDB_ACCOUNT_READ_LOGIN_SESSION	= 0,			// accountdb ���� �α��� ����Ű �� ���� Ȯ�� (�α��μ������� ���)
	enDB_ACCOUNT_READ_USER,							// accountdb ���� ȸ�� ���� ��� & status login ���·�

	enDB_ACCOUNT_WRITE_STATUS_LOGOUT				// �÷��̾� �α׾ƿ��� status �� �α׾ƿ� ���·� ����
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
	int			Party;					// ��Ƽ 1 / 2
};



// enDB_ACCOUNT_WRITE_STATUS_LOGOUT ----------------------------------------------------------

struct stDB_ACCOUNT_WRITE_STATUS_LOGOUT_in
{
	__int64		AccountNo;
};




//--------------------------------------------------------
// DB ���� ������� �޽���
//
//--------------------------------------------------------
#define dfDBWRITER_MSG_MAX	200

#define dfDBWRITER_TYPE_ACCOUNT		1		// AccountDB 
#define dfDBWRITER_TYPE_GAME		2		// GameDB
#define dfDBWRITER_TYPE_HEARTBEAT	3		// DBThread Heartbeat

//--------------------------------------------------------
// DB ���� �޽��� ���պ�.
//--------------------------------------------------------
typedef struct _st_DBWRITER_MSG
{
	int					Type_DB;			// AccountDB, GameDB ����	
	en_DB_ACTION_TYPE	Type_Message;		// Message ����

	BYTE	Message[dfDBWRITER_MSG_MAX];	// �޽��� ������ �� ������ ����
											// �Ʒ��� ����ü���� ��� ���� �� �� �ִ� ������.
											// �������� ����Ѵ�.
} st_DBWRITER_MSG;






// ��� DB ���� ���� ������ DBWrite ���� �����ؾ� �Ѵ�.
//
// DB ���� �޽����� ���̺� ������ �ƴ� ������ ������ ���°��� �´�.
// �ϳ��� �޽��� ó���� ���� ������ Ʈ������ �ɾ ó�� �ؾ� ��.
// �׷��Ƿ� �ϳ��� st_DBWRITER_MSG �޽����� �ϳ��� ������ �ؾ� �ϴ°��� �ƴ�.

