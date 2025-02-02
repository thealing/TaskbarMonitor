#include "Counters.h"

#include "Winternl.h"

static int CpuUsage;

static int MemoryUsage;

static int DownloadSpeed;

static int UploadSpeed;

static int DiskReadSpeed;

static int DiskWriteSpeed;

static void UpdateCpuCounter()
{
	static ULONGLONG LastIdleTime;

	static ULONGLONG LastKernelTime;

	static ULONGLONG LastUserTime;

	ULONGLONG IdleTime;

	ULONGLONG KernelTime;

	ULONGLONG UserTime;

	if (GetSystemTimes((PFILETIME)&IdleTime, (PFILETIME)&KernelTime, (PFILETIME)&UserTime) == FALSE)
	{
		LogMessage(LOG_WARNING, _T("Failed to get processor times."));

		return;
	}

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

static void UpdateMemoryCounter()
{
	MEMORYSTATUSEX MemoryStatus = { sizeof(MemoryStatus) };

	if (GlobalMemoryStatusEx(&MemoryStatus) == FALSE)
	{
		LogMessage(LOG_WARNING, _T("Failed to get memory status."));

		return;
	}

	MemoryUsage = MemoryStatus.dwMemoryLoad;
}

static void UpdateNetworkCounters()
{
	static int LastUploadedBytes;

	static int LastDownloadedBytes;

	ULONG InterfaceCount = 0;

	if (GetIfTable(NULL, &InterfaceCount, FALSE) != ERROR_INSUFFICIENT_BUFFER)
	{
		LogMessage(LOG_WARNING, _T("Failed to query interface count."));

		return;
	}

	MIB_IFTABLE* Table = malloc(sizeof(MIB_IFTABLE) + InterfaceCount * sizeof(MIB_IFROW));

	if (Table == NULL)
	{
		LogMessage(LOG_WARNING, _T("Failed to allocate the interface table."));

		return;
	}

	if (GetIfTable(Table, &InterfaceCount, FALSE) == NO_ERROR)
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
	else
	{
		LogMessage(LOG_WARNING, _T("Failed to get the interface table."));
	}
	

	free(Table);
}

static void UpdateDiskCounters()
{
	static ULONGLONG LastReadAmount;

	static ULONGLONG LastWriteAmount;

	HMODULE NtDllModule = LoadLibrary(_T("ntdll.dll"));

	if (NtDllModule == NULL)
	{
		LogMessage(LOG_WARNING, _T("Failed to load ntdll module."));

		return;
	}

	PNtQuerySystemInformation NtQuerySystemInformation = (PNtQuerySystemInformation)GetProcAddress(NtDllModule, "NtQuerySystemInformation");

	if (NtQuerySystemInformation != NULL)
	{
		SYSTEM_PERFORMANCE_INFORMATION PerformanceInformation = { 0 };

		if (NtQuerySystemInformation(SystemPerformanceInformation, &PerformanceInformation, sizeof(PerformanceInformation), NULL) == ERROR_SUCCESS)
		{
			ULONGLONG ReadAmount = PerformanceInformation.ReadTransferCount.QuadPart;

			ULONGLONG WriteAmount = PerformanceInformation.WriteTransferCount.QuadPart;

			DiskReadSpeed = (int)(ReadAmount - LastReadAmount);

			DiskWriteSpeed = (int)(WriteAmount - LastWriteAmount);

			LastReadAmount = ReadAmount;

			LastWriteAmount = WriteAmount;
		}
		else
		{
			LogMessage(LOG_WARNING, _T("Failed to get query the system performance information."));
		}
	}
	else
	{
		LogMessage(LOG_WARNING, _T("Failed to get the address of NtQuerySystemInformation."));
	}

	FreeLibrary(NtDllModule);
}

void InitCounters()
{
	UpdateCounters();

	UpdateCounters();
}

void UpdateCounters()
{
	UpdateCpuCounter();

	UpdateMemoryCounter();

	UpdateNetworkCounters();

	UpdateDiskCounters();
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

int GetDiskReadSpeed()
{
	return DiskReadSpeed; 
}

int GetDiskWriteSpeed()
{
	return DiskWriteSpeed; 
}
