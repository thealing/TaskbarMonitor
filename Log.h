#pragma once

#include "Header.h"

enum LOG_LEVEL
{
	LOG_INFO,

	LOG_WARNING,

	LOG_ERROR,

	LOG_COUNT
};

void LogSetFile(const TCHAR* Path);

void LogMessage(LOG_LEVEL Level, const TCHAR* Format, ...);
