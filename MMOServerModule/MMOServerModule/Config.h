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
};

extern CConfig g_Config;