#pragma once

class CConfig
{
public:
	// Network Block
	wchar_t szServerGroupName[32];			// 서버 이름

	wchar_t szNetBindIP[16];				// Bind할 Net서버 IP주소
	int		iNetBindPort;					// Bind할 Net서버 Port번호
	int		iNetThreadNum;					// Net서버 thread 개수
	bool	bNetNagleOpt;					// Net서버 Nagle 옵션 여부

	wchar_t szLanBindIP[16];				// Bind할 Lan서버 IP주소
	int		iLanBindPort;					// Bind할 Lan서버 Port번호
	int		iLanThreadNum;					// Lan서버 thread 개수
	bool	bLanNagleOpt;					// Lan서버 Nagle 옵션 여부

	wchar_t szLoginServerIP[16];			// 로그인 서버에 연결할 IP주소
	int		iLoginServerPort;				// 로그인 서버에 연결할 Port번호

	wchar_t szMonitoringServerIP[16];		// 모니터링 서버에 연결할 IP주소
	int		iMonitoringServerPort;			// 모니터링 서버에 연결할 Port번호

	// System Block
	int		iClientMax;						// 접속받을 최대 클라이언트 수
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