#pragma once

#define STATUS_SUCCESS 0x00000000
#define Print(a, ...) printf("%s: " a "\n", __FUNCTION__, ##__VA_ARGS__)

#define WaitCondition(condition) \
	while(condition) \
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

#define WaitConditionWithTimeout(condition, timeout_ms) \
{ \
	auto start_time = std::chrono::steady_clock::now(); \
	while(condition) \
	{ \
		std::this_thread::sleep_for(std::chrono::milliseconds(1)); \
		auto current_time = std::chrono::steady_clock::now(); \
		auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count(); \
		if (elapsed_time >= timeout_ms) \
		{ \
			break; \
		} \
	} \
}

typedef enum _SECTION_INHERIT
{
	ViewShare = 1,
	ViewUnmap = 2

} SECTION_INHERIT;

#pragma comment(lib, "ntdll.lib")

EXTERN_C NTSTATUS NTAPI RtlAdjustPrivilege(ULONG, BOOLEAN, BOOLEAN, BOOLEAN*);
EXTERN_C NTSTATUS NTAPI NtRaiseHardError(NTSTATUS, ULONG, ULONG, PULONG_PTR, ULONG, PULONG);
EXTERN_C NTSTATUS NTAPI NtSuspendProcess(HANDLE);
EXTERN_C NTSTATUS NTAPI NtResumeProcess(HANDLE);
EXTERN_C NTSTATUS NTAPI NtSetDebugFilterState(DWORD, DWORD, BOOLEAN);
EXTERN_C NTSTATUS NTAPI ZwCreateSection(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
EXTERN_C NTSTATUS NTAPI ZwMapViewOfSection(HANDLE, HANDLE, PVOID*, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, SECTION_INHERIT, ULONG, ULONG);
EXTERN_C NTSTATUS NTAPI ZwUnmapViewOfSection(HANDLE, PVOID);

class Remap
{
private:
	inline static bool RemapViewOfSection(HANDLE ProcessHandle, PVOID BaseAddress, SIZE_T RegionSize, DWORD NewProtection, PVOID CopyBuffer) 
	{
		SIZE_T numberOfBytesRead = 0;
		if (ReadProcessMemory(ProcessHandle, BaseAddress, CopyBuffer, RegionSize, &numberOfBytesRead) == FALSE) 
		{
			return false;
		}

		HANDLE hSection = NULL;
		LARGE_INTEGER sectionMaxSize = {};
		sectionMaxSize.QuadPart = RegionSize;

		NTSTATUS R = ZwCreateSection(&hSection, SECTION_ALL_ACCESS, NULL, &sectionMaxSize, PAGE_EXECUTE_READWRITE, SEC_COMMIT, NULL);

		if (R != STATUS_SUCCESS) 
		{
			return false;
		}

		NTSTATUS R2 = ZwUnmapViewOfSection(ProcessHandle, BaseAddress);

		if (R2 != STATUS_SUCCESS) 
		{
			return false;
		}

		PVOID viewBase = BaseAddress;
		LARGE_INTEGER sectionOffset = {};
		SIZE_T viewSize = 0;

		NTSTATUS R3 = ZwMapViewOfSection(hSection, ProcessHandle, &viewBase, 0, RegionSize, &sectionOffset, &viewSize, ViewUnmap, 0, NewProtection);

		if (R3 != STATUS_SUCCESS) 
		{
			return false;
		}

		SIZE_T numberOfBytesWritten2 = 0;

		if (WriteProcessMemory(ProcessHandle, viewBase, CopyBuffer, viewSize, &numberOfBytesWritten2) == FALSE) 
		{
			return false;
		}

		if (CloseHandle(hSection) == FALSE) 
		{
			return false;
		}

		return true;
	}

public:
	inline static bool PatchSection(HANDLE hProcess, PVOID regionBase, SIZE_T regionSize, DWORD newProtection)
	{
		PVOID EmptyAlloc = VirtualAlloc(NULL, regionSize, MEM_COMMIT | MEM_RESERVE, newProtection);

		if (EmptyAlloc == NULL)
		{
			return false;
		}

		if (RemapViewOfSection(hProcess, regionBase, regionSize, newProtection, EmptyAlloc) == false)
		{
			return false;
		}

		if (VirtualFree(EmptyAlloc, NULL, MEM_RELEASE) == FALSE)
		{
			return false;
		}

		return true;
	}
};

