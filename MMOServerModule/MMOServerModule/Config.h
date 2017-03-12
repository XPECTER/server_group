#pragma once

class CConfig
{
public:
	// Network Block
	wchar_t szServerGroupName[32];			// ���� �̸�

	wchar_t szNetBindIP[16];				// Bind�� Net���� IP�ּ�
	int		iNetBindPort;					// Bind�� Net���� Port��ȣ
	int		iNetThreadNum;					// Net���� thread ����
	bool	bNetNagleOpt;					// Net���� Nagle �ɼ� ����

	wchar_t szLanBindIP[16];				// Bind�� Lan���� IP�ּ�
	int		iLanBindPort;					// Bind�� Lan���� Port��ȣ
	int		iLanThreadNum;					// Lan���� thread ����
	bool	bLanNagleOpt;					// Lan���� Nagle �ɼ� ����

	wchar_t szLoginServerIP[16];			// �α��� ������ ������ IP�ּ�
	int		iLoginServerPort;				// �α��� ������ ������ Port��ȣ

	wchar_t szMonitoringServerIP[16];		// ����͸� ������ ������ IP�ּ�
	int		iMonitoringServerPort;			// ����͸� ������ ������ Port��ȣ

	wchar_t szAgentServerIP[16];			// ������Ʈ ������ ������ IP�ּ�
	int		iAgentServerPort;				// ������Ʈ ������ ������ Port��ȣ

	// System Block
	int		iClientMax;						// ���ӹ��� �ִ� Ŭ���̾�Ʈ ��
	int		iPacketCode;
	int		iPacketKey1;
	int		iPacketKey2;

	// LOG Block
	int		iLogLevel;
	bool	bPrintConsole;
	bool	bPrintFile;
	bool	bPrintDatabase;

	// DATABASE Block
	char	szAccountDBIP[16];
	int		iAccountDBPort;
	char	szAccountDBUser[32];
	char	szAccountDBPassword[32];
	char	szAccountDBName[32];

	char	szGameDBIP[16];
	int		iGameDBPort;
	char	szGameDBUser[32];
	char	szGameDBPassword[32];
	char	szGameDBName[32];

	char	szLogDBIP[16];
	int		iLogDBPort;
	char	szLogDBUser[32];
	char	szLogDBPassword[32];
	char	szLogDBName[32];
};

extern CConfig g_Config;