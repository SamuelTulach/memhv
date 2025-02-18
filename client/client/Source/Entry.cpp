#include <Windows.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <iostream>
#include <chrono>
#include <mutex>

#include "../Library/memhv.h"

typedef enum _MEMORY_INFORMATION_CLASS
{
    MemoryBasicInformation, // MEMORY_BASIC_INFORMATION
    MemoryWorkingSetInformation, // MEMORY_WORKING_SET_INFORMATION
    MemoryMappedFilenameInformation, // UNICODE_STRING
    MemoryRegionInformation, // MEMORY_REGION_INFORMATION
    MemoryWorkingSetExInformation, // MEMORY_WORKING_SET_EX_INFORMATION
    MemorySharedCommitInformation, // MEMORY_SHARED_COMMIT_INFORMATION
    MemoryImageInformation,
    MemoryRegionInformationEx,
    MemoryPrivilegedBasicInformation,
    MemoryEnclaveImageInformation,
    MemoryBasicInformationCapped
} MEMORY_INFORMATION_CLASS;

EXTERN_C NTSYSAPI NTSTATUS NTAPI NtQueryVirtualMemory(HANDLE processHandle, void* baseAddress, MEMORY_INFORMATION_CLASS memoryInformationClass, void* memoryInformation, size_t memoryInformationLength, size_t* returnLength);

ULONG64 GetModule(UINT32 pid, const wchar_t* moduleName)
{
    HANDLE targetProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION, 0, pid);
    if (!targetProcessHandle || targetProcessHandle == INVALID_HANDLE_VALUE)
        return 0;

    ULONG64 currentAddress = 0;
    MEMORY_BASIC_INFORMATION memoryInformation;
    while (VirtualQueryEx(targetProcessHandle, reinterpret_cast<void*>(currentAddress), &memoryInformation, sizeof(MEMORY_BASIC_INFORMATION64)))
    {
        if (memoryInformation.Type == MEM_MAPPED || memoryInformation.Type == MEM_IMAGE)
        {
            constexpr size_t bufferSize = 1024;
            void* buffer = malloc(bufferSize);

            size_t bytesOut;
            NTSTATUS status = NtQueryVirtualMemory(targetProcessHandle, memoryInformation.BaseAddress, MemoryMappedFilenameInformation, buffer, bufferSize, &bytesOut);
            if (status == 0)
            {
                UNICODE_STRING* stringBuffer = static_cast<UNICODE_STRING*>(buffer);
                if (wcsstr(stringBuffer->Buffer, moduleName) && !wcsstr(stringBuffer->Buffer, L".mui"))
                {
                    free(buffer);
                    CloseHandle(targetProcessHandle);
                    return reinterpret_cast<ULONG64>(memoryInformation.BaseAddress);
                }
            }

            free(buffer);
        }

        currentAddress = reinterpret_cast<ULONG64>(memoryInformation.BaseAddress) + memoryInformation.RegionSize;
    }

    CloseHandle(targetProcessHandle);
    return 0;
}

UINT32 LookupProcessId(const wchar_t* processName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot)
    {
        PROCESSENTRY32 entry = { 0 };
        entry.dwSize = sizeof(entry);
        if (Process32First(snapshot, &entry))
        {
            do
            {
                if (0 == _wcsicmp(entry.szExeFile, processName))
                {
                    CloseHandle(snapshot);
                    return entry.th32ProcessID;
                }
            } while (Process32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
    }

    return 0;
}

std::mutex m;
ULONG64 MainModule = 0;
void ThreadBench(int id)
{
    while (true)
    {
        UINT64 totalOk = 0;
        UINT64 totalFail = 0;

        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 100000; i++)
        {
            int offset = rand() % 20 + 1;
            offset += 0x48;
            volatile int readValue = HV::Read<int>(MainModule + offset);
            volatile int readConfirm = HV::Read<int>(MainModule + offset);
            if (readValue == readConfirm && readValue != 0)
                totalOk++;
            else
            {
                totalFail++;
                m.lock();
                printf("[!] Invalid read: %x %x\n", readValue, readConfirm);
                m.unlock();
            }
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        m.lock();
        printf("[+] Ok: %llu Fail: %llu In: %llu\n", totalOk, totalFail, duration);
        m.unlock();
    }
}

int main()
{
    printf("[>] Checking presence...\n");
    bool status = HV::CheckPresence();
    if (!status)
    {
        printf("[!] Hypervisor not running\n");
        getchar();
        return EXIT_FAILURE;
    }

    printf("[>] Instructing hypervisor to protect itself...\n");
    status = HV::Protect();
    if (!status)
    {
        printf("[!] Hypervisor self-protection failed\n");
        getchar();
        return EXIT_FAILURE;
    }

    printf("[>] Searching for target process...\n");
    UINT32 targetProcessId = LookupProcessId(L"SystemInformer.exe");
    if (!targetProcessId)
    {
        printf("[!] Process not found\n");
        getchar();
        return EXIT_FAILURE;
    }

    printf("[+] Process has PID of %u\n", targetProcessId);

    printf("[>] Attaching to process...\n");
    status = HV::AttachToProcess(targetProcessId);
    if (!status)
    {
        printf("[!] Failed to attach\n");
        getchar();
        return EXIT_FAILURE;
    }

    printf("[+] Current process: EPROCESS -> 0x%llx CR3 -> 0x%llx\n", HV::Data::CurrentEPROCESS, HV::Data::CurrentDirectoryBase);
    printf("[+] Target process: EPROCESS -> 0x%llx CR3 -> 0x%llx\n", HV::Data::TargetEPROCESS, HV::Data::TargetDirectoryBase);

    printf("[>] Getting module base address...\n");
    MainModule = GetModule(targetProcessId, L"SystemInformer.exe");
    if (!MainModule)
    {
        printf("[!] Failed to get module base address\n");
        getchar();
        return EXIT_FAILURE;
    }

    printf("[+] Module is at 0x%llx\n", MainModule);

    printf("[>] Reading module header...\n");
    UINT64 header = HV::Read<UINT64>(MainModule);
    if (!header)
    {
        printf("[!] Failed to read header\n");
        getchar();
        return EXIT_FAILURE;
    }

    printf("[+] Header data: 0x%p\n", reinterpret_cast<void*>(header));

    printf("[>] Press any key to start multi-threaded test...\n");
    getchar();

    printf("[>] Multi-thread test...\n");
    for (int i = 0; i < 5; i++)
    {
        printf("[+] Starting ID %i...\n", i);
        std::thread thread(ThreadBench, i);
        thread.detach();
        Sleep(10);
    }

    getchar();
    return EXIT_SUCCESS;
}