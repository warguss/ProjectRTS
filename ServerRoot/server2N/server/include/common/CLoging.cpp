#include "CLoging.h"
#include "CThreadLockManager.h"

CLoging g_logger;
CLoging::CLoging()
{
	_mutex = PTHREAD_MUTEX_INITIALIZER;
	_cond = PTHREAD_COND_INITIALIZER;
	_fp = NULL;
	init();
} 

void CLoging::clear()
{
	/******************
	 * AutoLock
	 ******************/
	CThreadLockManager autoLock(&_mutex, &_cond);
	if ( _fp )
	{
		fclose(_fp);
		_fp = NULL;
	} 
} 


CLoging::~CLoging()
{
	clear();
}

void CLoging::init()
{
	/******************
	 * AutoLock
	 ******************/
	CThreadLockManager autoLock(&_mutex, &_cond);
	if ( !_fp )
	{
		_fp = fopen("./log/debug.log", "ar+");
	}
} 

bool CLoging::wlog(int lv, const char* file, int line, const char* fmt, va_list args)
{
	/******************
	 * AutoLock
	 ******************/
	CThreadLockManager autoLock(&_mutex, &_cond);

	/******************
	 * fp Set
	 ******************/
	FILE *fp = _fp;
	char pMsg[8192];
	va_list copied;
	time_t cur;
	time(&cur);

	struct tm *t = localtime(&cur);
	char pTime[64];
	memset(pTime, '\0', sizeof(pTime));
	sprintf(pTime, "%04d-%02d-%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	char pType[10];
	memset(pType, '\0', sizeof(pType));
	switch(lv)
	{
		case Trace:
			strcpy(pType, "TRC");
			break;
		case Debug:
			strcpy(pType, "DBG");
			break;
		case Info:
			strcpy(pType, "INF");
			break;
		case Warn:
			strcpy(pType, "WRN");
			break;
		case Error:
			strcpy(pType, "ERR");
			break;
		case Fatal:
			strcpy(pType, "FTL");
			break;
	}
	va_copy(copied, args);
	vsnprintf(pMsg, sizeof(pMsg), fmt, copied);
	//printf("%s|%s|%s(%d):%s\n",  pType, pTime, file, line, pMsg);
	if ( fp )
	{
		fprintf(fp, "%s|%s|%s(%d):%s\n",  pType, pTime, file, line, pMsg);
		fflush(fp);
	}

	va_end(copied);
}
