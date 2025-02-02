#include "Log.h"

static FILE* LogFile;

void LogSetFile(const TCHAR* Path)
{

#ifndef _DEBUG

	if (LogFile != NULL)
	{
		fclose(LogFile);
	}

	if (Path == NULL)
	{
		LogFile = NULL;
	}
	else
	{
		LogFile = _tfopen(Path, _T("w"));
	}

#endif

}

void LogMessage(LOG_LEVEL Level, const TCHAR* Format, ...)
{
	static const TCHAR* Labels[LOG_COUNT] = { _T("INFO "), _T("WARN "), _T("ERROR") };

	va_list ArgumentList;

	va_start(ArgumentList, Format);

	FILE* File = LogFile != NULL ? LogFile : stdout;

	_ftprintf(File, _T("%s | "), Labels[Level]);

	_vftprintf(File, Format, ArgumentList);

	_ftprintf(File, _T("\n"));

	fflush(File);

	va_end(ArgumentList);
}
