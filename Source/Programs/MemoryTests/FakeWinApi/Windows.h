#pragma once
#include <cstddef>
#include <cstdint>
using DWORD = std::uint32_t;
using BOOL = int;
using HANDLE = void*;
struct SYSTEM_INFO { DWORD dwPageSize; DWORD dwAllocationGranularity; };
inline constexpr DWORD MEM_RESERVE = 0x2000;
inline constexpr DWORD MEM_COMMIT = 0x1000;
inline constexpr DWORD MEM_RELEASE = 0x8000;
inline constexpr DWORD PAGE_READWRITE = 4;
inline constexpr DWORD ERROR_NOT_ENOUGH_MEMORY = 8;
inline constexpr DWORD ERROR_OUTOFMEMORY = 14;
inline constexpr DWORD ERROR_COMMITMENT_LIMIT = 1455;
inline constexpr DWORD STD_ERROR_HANDLE = static_cast<DWORD>(-12);
#define INVALID_HANDLE_VALUE reinterpret_cast<HANDLE>(static_cast<std::intptr_t>(-1))
void GetSystemInfo(SYSTEM_INFO* Info);
void* VirtualAlloc(void* Base, std::size_t Size, DWORD Flags, DWORD Protection);
BOOL VirtualFree(void* Base, std::size_t Size, DWORD Flags);
DWORD GetLastError();
void OutputDebugStringA(const char* Message);
HANDLE GetStdHandle(DWORD Which);
BOOL WriteFile(HANDLE File, const void* Data, DWORD Size, DWORD* Written, void* Overlapped);
HANDLE GetCurrentProcess();
BOOL TerminateProcess(HANDLE Process, unsigned int ExitCode);