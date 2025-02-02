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
}

static void UpdateMemoryCounter()
{
	MEMORYSTATUSEX MemoryStatus = { sizeof(MemoryStatus) };

	if (GlobalMemoryStatusEx(&MemoryStatus))
	{
		MemoryUsage = MemoryStatus.dwMemoryLoad;
	}
}

static void UpdateNetworkCounters()
{
	static int LastUploadedBytes;

	static int LastDownloadedBytes;

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
}

static void UpdateDiskCounters()
{
	static ULONGLONG LastReadAmount;

	static ULONGLONG LastWriteAmount;

	HMODULE NtDllModule = LoadLibrary(_T("ntdll.dll"));

	if (NtDllModule != NULL)
	{
		PNtQuerySystemInformation NtQuerySystemInformation = (PNtQuerySystemInformation)GetProcAddress(NtDllModule, "NtQuerySystemInformation");

		if (NtQuerySystemInformation)
		{
			SYSTEM_PERFORMANCE_INFORMATION spi = { 0 };

			if (NtQuerySystemInformation(SystemPerformanceInformation, &spi, sizeof(spi), NULL) == ERROR_SUCCESS)
			{
				ULONGLONG ReadAmount = spi.ReadTransferCount.QuadPart;

				ULONGLONG WriteAmount = spi.WriteTransferCount.QuadPart;

				DiskReadSpeed = (int)(ReadAmount - LastReadAmount);

				DiskWriteSpeed = (int)(WriteAmount - LastWriteAmount);

				LastReadAmount = ReadAmount;

				LastWriteAmount = WriteAmount;
			}
		}

		FreeLibrary(NtDllModule);
	}
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
