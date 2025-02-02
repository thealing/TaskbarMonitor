#include "Counters.h"

static ULONGLONG LastIdleTime;

static ULONGLONG LastKernelTime;

static ULONGLONG LastUserTime;

static int LastUploadedBytes;

static int LastDownloadedBytes;

static int CpuUsage;

static int MemoryUsage;

static int DownloadSpeed;

static int UploadSpeed;

void InitCounters()
{
	UpdateCounters();

	UpdateCounters();
}

void UpdateCounters()
{
	ULONGLONG IdleTime;
	
	ULONGLONG KernelTime;
	
	ULONGLONG UserTime;

	if (GetSystemTimes((PFILETIME)&IdleTime, (PFILETIME)&KernelTime, (PFILETIME)&UserTime))
	{
		ULONGLONG ElapsedTime = (KernelTime - LastKernelTime) + (UserTime - LastUserTime);

		ULONGLONG UsedTime = ElapsedTime - (IdleTime - LastIdleTime);

		if (ElapsedTime > 0)
		{
			CpuUsage = (int)(100 * UsedTime / ElapsedTime);

			LastIdleTime = IdleTime;

			LastKernelTime = KernelTime;

			LastUserTime = UserTime;
		}
	}

	ULONG InterfaceCount = 0;

	if (GetIfTable(NULL, &InterfaceCount, FALSE) == ERROR_INSUFFICIENT_BUFFER)
	{
		MIB_IFTABLE* Table = malloc(sizeof(MIB_IFTABLE) + InterfaceCount * sizeof(MIB_IFROW));

		if (Table != NULL && GetIfTable(Table, &InterfaceCount, FALSE) == NO_ERROR)
		{
			DWORD DownloadedBytes = 0;

			DWORD UploadedBytes = 0;

			for (ULONG i = 0; i < Table->dwNumEntries; i++)
			{
				MIB_IFROW* Row = &Table->table[i];

				if (Row->dwOperStatus < IF_OPER_STATUS_DISCONNECTED)
				{
					continue;
				}

				DownloadedBytes += Row->dwInOctets / 8;

				UploadedBytes += Row->dwOutOctets / 8;
			}

			DownloadSpeed = (int)(DownloadedBytes - LastDownloadedBytes);

			UploadSpeed = (int)(UploadedBytes - LastUploadedBytes);

			LastDownloadedBytes = DownloadedBytes;

			LastUploadedBytes = UploadedBytes;
		}

		free(Table);
	}

	MEMORYSTATUSEX MemoryStatus = { sizeof(MemoryStatus) };

	if (GlobalMemoryStatusEx(&MemoryStatus))
	{
		MemoryUsage = MemoryStatus.dwMemoryLoad;
	}
}

int GetCpuUsage()
{
	return CpuUsage;
}

int GetMemoryUsage()
{
	return MemoryUsage;
}

int GetDownloadSpeed()
{
	return DownloadSpeed;
}

int GetUploadSpeed()
{
	return UploadSpeed;
}
