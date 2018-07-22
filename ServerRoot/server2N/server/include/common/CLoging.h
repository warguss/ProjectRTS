#ifndef _CLOGGING_H_
#define _CLOGGING_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <iostream>

using namespace std;

class CLoging
{
	public:
		enum LogLevel
		{
			Trace = 7, 
			Warn = 6,
			Debug = 5,
			Fail = 4,
			Info = 3,
			Error = 2,
			Fatal = 1,
			Success = 0
		};

		FILE *_fp;
		bool _lineFlush;
		pthread_mutex_t _mutex;
		pthread_cond_t _cond;
	public:
		CLoging(const char* fileName);
		CLoging();
		~CLoging();

		void init();
		void clear();

		inline void debug(const char *file, int line, const char* fmt, ...) 
		{
			va_list args;
			va_start(args, fmt);
			wlog(Debug, file, line, fmt, args);
			va_end(args);
		}

		inline void info(const char *file, int line, const char* fmt, ...) 
		{
			va_list args;
			va_start(args, fmt);
			wlog(Info, file, line, fmt, args);
			va_end(args);
		}

		inline void warn(const char *file, int line, const char* fmt, ...) 
		{
			va_list args;
			va_start(args, fmt);
			wlog(Warn, file, line, fmt, args);
			va_end(args);
		}

		inline void error(const char *file, int line, const char* fmt, ...) 
		{
			va_list args;
			va_start(args, fmt);
			wlog(Error, file, line, fmt, args);
			va_end(args);
		}

		inline void fatal(const char *file, int line, const char* fmt, ...) 
		{
			va_list args;
			va_start(args, fmt);
			wlog(Fatal, file, line, fmt, args);
			va_end(args);
		}

		inline void log(int lv, const char *file, int line, const char* fmt, ...) 
		{
			va_list args;
			va_start(args, fmt);
			wlog(lv, file, line, fmt, args);
			va_end(args);
		}

		bool wlog(int lv, const char* file, int line, const char* fmt, va_list args);
};

#endif
