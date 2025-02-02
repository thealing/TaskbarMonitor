#pragma once

#include "Log.h"

void InitCounters();

void UpdateCounters();

int GetCpuUsage();

int GetMemoryUsage();

int GetDownloadSpeed();

int GetUploadSpeed();

int GetDiskReadSpeed();

int GetDiskWriteSpeed();
