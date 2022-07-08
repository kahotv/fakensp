#pragma once

#if LOG_TYPE  == 0
#define TRACEX_(pFmt,...)
#elif LOG_TYPE ==1
inline void  TRACEX_(const char* pFmt, ...)
{
	CHAR s_log[8192];

	va_list arg;
	va_start(arg, pFmt);
	_vsnprintf_s(s_log, sizeof(s_log), pFmt, arg);
	va_end(arg);

	printf("%s", s_log);

	return;
};
inline void  TRACEX_(const wchar_t* pFmt, ...)
{
	WCHAR s_log[8192];

	va_list arg;
	va_start(arg, pFmt);
	_vsnwprintf_s(s_log, sizeof(s_log) / 2, pFmt, arg);
	va_end(arg);

	wprintf(s_log);

	return;
};
#elif LOG_TYPE ==2
inline void  TRACEX_(const char* pFmt, ...)
{
	CHAR s_log[8192];

	va_list arg;
	va_start(arg, pFmt);
	_vsnprintf_s(s_log, sizeof(s_log), pFmt, arg);
	va_end(arg);

	OutputDebugStringA(s_log);

	return;
};
inline void  TRACEX_(const wchar_t* pFmt, ...)
{
	WCHAR s_log[8192];

	va_list arg;
	va_start(arg, pFmt);
	_vsnwprintf_s(s_log, sizeof(s_log) / 2, pFmt, arg);
	va_end(arg);

	OutputDebugStringW(s_log);

	return;
};
#endif