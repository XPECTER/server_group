#include "Log.h"

bool LOG::bConsole = false;
bool LOG::bFile = true;
bool LOG::bDatabase = false;
int LOG::iLogLevel;
 __int64 LOG::logNo = 0;
SRWLOCK LOG::_srwLock;
CRITICAL_SECTION LOG::_cs;
wchar_t g_szLogBuff[1024] = { 0, };
LOG SysLog;

//LOG* LOG::_instance = nullptr;
//SRWLOCK LOG::_createSrwLock;

LOG::LOG()
{
	this->logNo = 0;
	
	//memset(this->_szFolderName, 0, sizeof(wchar_t) * dfDIRECTORY_LEN);
	InitializeSRWLock(&this->_srwLock);
	InitializeCriticalSection(&this->_cs);
}

LOG::~LOG()
{

}

//LOG* LOG::GetInstance(void)
//{
//	AcquireSRWLockExclusive(&_srwLock);
//	if (nullptr == LOG::_instance)
//	{
//		LOG::_instance = new LOG;
//	}
//	ReleaseSRWLockExclusive(&_srwLock);
//
//	return LOG::_instance;
//}

bool LOG::connectDatabase(void)
{
#ifdef MYSQL
	mysql_init(&conn);

	connector = mysql_real_connect(&conn, "localhost", "root", "menistream@2460", "log", 3306, (char *)NULL, 0);
#else
	return false;
#endif
}

void LOG::printLog(wchar_t *category, int logLevel, wchar_t *szLogString)//, ...)
{
	tm t;
	wchar_t buf[1024] = { 0, };
	time_t unixTime = time(NULL);
	localtime_s(&t, &unixTime);
	
	__int64 iLogNo = InterlockedIncrement64(&logNo);
	int buffSize = 0;

	wchar_t szLevel[10] = { 0, };

	switch (logLevel)
	{
	case LEVEL_DEBUG:
		{
			swprintf_s(szLevel, L"DEBUG");
			break;
		}
	case LEVEL_WARNING:
		{
			swprintf_s(szLevel, L"WARNG");
			break;
		}
	case LEVEL_ERROR:
		{
			swprintf_s(szLevel, L"ERROR");
			break;
		}
	}

	swprintf_s(buf, 1024, L"[%s][%04d-%02d-%02d %02d:%02d:%02d / %s / %010d]%s",
		category, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, szLevel, iLogNo, szLogString);
	
	
	//va_list vl;
	/*va_start(vl, fmt);
	StringCchVPrintf(buf + buffSize, 1024 - buffSize, fmt, vl);
	va_end(vl);*/
	
	
	if (true == bConsole)
	{
		wprintf_s(L"%s\n", buf);
	}

	if (true == bFile)
	{
		EnterCriticalSection(&LOG::_cs);

		FILE *pf = nullptr;
		WCHAR filename[256] = { 0, };
		
		swprintf_s(filename, L"LOG\\%04d%02d_%s.txt", t.tm_year + 1900, t.tm_mon + 1, category);
		if (0 == _wfopen_s(&pf, filename, L"a, ccs=UNICODE"))
		{
			fputws(buf, pf);
			fputws(L"\n", pf);
			fclose(pf);
		}

		LeaveCriticalSection(&LOG::_cs);
	}

#ifdef MYSQL
	if (true == bDatabase)
	{
		char query[256] = { 0, };
		sprintf_s(query, "INSERT INTO ....");
		mysql_query(connector, query);

		// 결과에 따라서 테이블 만들고 다시 넣고 하면 됨.
	}
#endif
	
	return;
}