#pragma once
//#define MYSQL

#include <Windows.h>
#include <ctime>
#include <stdarg.h>
#include <strsafe.h>
#include <direct.h>

#ifdef MYSQL
#include <mysql.h>
#include <my_global.h>
#endif

#define dfDIRECTORY_LEN 128
#define ON_LOG

#ifdef ON_LOG
#define SYSLOG(Category, LogLevel, fmt, ...)				\
do{															\
	if (LOG::iLogLevel <= LogLevel)							\
		{													\
		swprintf_s(g_szLogBuff, 1024, fmt, ##__VA_ARGS__);	\
		LOG::printLog(Category, LogLevel, g_szLogBuff);		\
		}													\
} while (0)										

#define SYSLOG_SETDIRECTORY(FolderName) LOG::CreateFolder(FolderName)
#else
#define SYSLOG(Category, LogLevel, fmt, ...)
#endif

class LOG
{
public :
	enum define
	{
		LEVEL_DEBUG = 1,
		LEVEL_WARNING = 2,
		LEVEL_ERROR = 3
	};

public:
	LOG();
	~LOG();

public :
	//static LOG* GetInstance(void);
	static void printLog(WCHAR *category, int logLevel, wchar_t *szLogString);//, ...);
	static bool CreateFolder(wchar_t *szFolderName);
private:
	//static LOG* _instance;

	bool connectDatabase(void);
	
	
public:
	static bool bConsole;
	static bool bFile;
	static bool bDatabase;

	static int iLogLevel;
private :

	//static wchar_t *_szFolderName;
	static __int64 logNo;
	static SRWLOCK _srwLock;
	static CRITICAL_SECTION _cs;

	// mysql
#ifdef MYSQL
private:
	MYSQL *connector;
	MYSQL conn;
#endif
};

extern LOG SysLog;
extern wchar_t g_szLogBuff[1024];