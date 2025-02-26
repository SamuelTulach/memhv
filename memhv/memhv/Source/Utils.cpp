#include "Global.h"

char* Utils::Compare(const char* haystack, const char* needle)
{
    do
    {
        const char* h = haystack;
        const char* n = needle;
        while (tolower(static_cast<unsigned char>(*h)) == tolower(static_cast<unsigned char>(*n)) && *n)
        {
            h++;
            n++;
        }

        if (*n == 0)
            return const_cast<char*>(haystack);
    } while (*haystack++);
    return nullptr;
}

PVOID Utils::GetModuleBase(const char* moduleName)
{
    PVOID address = nullptr;
    ULONG size = 0;

    NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, &size, 0, &size);
    if (status != STATUS_INFO_LENGTH_MISMATCH)
        return nullptr;

#pragma warning(disable : 4996) // 'ExAllocatePool': ExAllocatePool is deprecated, use ExAllocatePool2
    PSYSTEM_MODULE_INFORMATION moduleList = static_cast<PSYSTEM_MODULE_INFORMATION>(ExAllocatePool(NonPagedPool, size));
    if (!moduleList)
        return nullptr;

    status = ZwQuerySystemInformation(SystemModuleInformation, moduleList, size, nullptr);
    if (!NT_SUCCESS(status))
        goto end;

    for (ULONG_PTR i = 0; i < moduleList->ulModuleCount; i++)
    {
        ULONG64 pointer = reinterpret_cast<ULONG64>(&moduleList->Modules[i]);
        pointer += sizeof(SYSTEM_MODULE);
        if (pointer > (reinterpret_cast<ULONG64>(moduleList) + size))
            break;

        SYSTEM_MODULE module = moduleList->Modules[i];
        module.ImageName[255] = '\0';
        if (Compare(module.ImageName, moduleName))
        {
            address = module.Base;
            break;
        }
    }

end:
    ExFreePool(moduleList);
    return address;
}

#define IN_RANGE(x, a, b) (x >= a && x <= b)
#define GET_BITS(x) (IN_RANGE((x&(~0x20)),'A','F')?((x&(~0x20))-'A'+0xA):(IN_RANGE(x,'0','9')?x-'0':0))
#define GET_BYTE(a, b) (GET_BITS(a) << 4 | GET_BITS(b))
ULONG64 Utils::FindPattern(void* baseAddress, const ULONG64 size, const char* pattern)
{
    BYTE* firstMatch = nullptr;
    const char* currentPattern = pattern;

    BYTE* start = static_cast<BYTE*>(baseAddress);
    const BYTE* end = start + size;

    for (BYTE* current = start; current < end; current++)
    {
        const BYTE byte = currentPattern[0]; if (!byte) return reinterpret_cast<ULONG64>(firstMatch);
        if (byte == '\?' || *static_cast<BYTE*>(current) == GET_BYTE(byte, currentPattern[1]))
        {
            if (!firstMatch) firstMatch = current;
            if (!currentPattern[2]) return reinterpret_cast<ULONG64>(firstMatch);
            ((byte == '\?') ? (currentPattern += 2) : (currentPattern += 3));
        }
        else
        {
            currentPattern = pattern;
            firstMatch = nullptr;
        }
    }

    return 0;
}

ULONG64 Utils::FindPatternImage(void* base, const char* pattern)
{
    ULONG64 match = 0;

    PIMAGE_NT_HEADERS64 headers = reinterpret_cast<PIMAGE_NT_HEADERS64>(reinterpret_cast<ULONG64>(base) + static_cast<PIMAGE_DOS_HEADER>(base)->e_lfanew);
    const PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(headers);
    for (USHORT i = 0; i < headers->FileHeader.NumberOfSections; ++i)
    {
        const PIMAGE_SECTION_HEADER section = &sections[i];
        if (memcmp(section->Name, ".text", 5) == 0 || *reinterpret_cast<DWORD32*>(section->Name) == 'EGAP')
        {
            match = FindPattern(reinterpret_cast<void*>(reinterpret_cast<ULONG64>(base) + section->VirtualAddress), section->Misc.VirtualSize, pattern);
            if (match)
                break;
        }
    }

    return match;
}

PVOID Utils::AllocateContiguousMemory(const SIZE_T size)
{
    PHYSICAL_ADDRESS boundary, lowest, highest;
    boundary.QuadPart = lowest.QuadPart = 0;
    highest.QuadPart = -1;

    const PVOID allocated = MmAllocateContiguousMemorySpecifyCacheNode(size, lowest, highest, boundary, MmNonCached, MM_ANY_NODE_OK);
    if (!allocated)
        KeBugCheck(HAL_MEMORY_ALLOCATION);

    RtlZeroMemory(allocated, size);

    return allocated;
}

NTSTATUS Utils::ExecuteOnEachProcessor(NTSTATUS(*callback)(PVOID), const PVOID context)
{
    const ULONG numOfProcessors = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
    for (ULONG i = 0; i < numOfProcessors; i++)
    {
        PROCESSOR_NUMBER processorNumber;
        NTSTATUS status = KeGetProcessorNumberFromIndex(i, &processorNumber);
        if (!NT_SUCCESS(status))
            return status;

        GROUP_AFFINITY affinity;
        affinity.Group = processorNumber.Group;
        affinity.Mask = 1ULL << processorNumber.Number;
        affinity.Reserved[0] = affinity.Reserved[1] = affinity.Reserved[2] = 0;

        GROUP_AFFINITY oldAffinity;
        KeSetSystemGroupAffinityThread(&affinity, &oldAffinity);

        status = callback(context);

        KeRevertToUserGroupAffinityThread(&oldAffinity);

        if (!NT_SUCCESS(status))
            return status;
    }

    return STATUS_SUCCESS;
}

extern "C" IMAGE_DOS_HEADER __ImageBase;
NTSTATUS Utils::GetCurrentDriverInfo(ULONG64* baseAddress, SIZE_T* size)
{
    if (__ImageBase.e_magic != IMAGE_DOS_SIGNATURE)
        return STATUS_INVALID_IMAGE_FORMAT;

    const PIMAGE_NT_HEADERS64 headers = reinterpret_cast<PIMAGE_NT_HEADERS64>(reinterpret_cast<ULONG64>(&__ImageBase) + __ImageBase.e_lfanew);
    if (headers->Signature != IMAGE_NT_SIGNATURE)
        return STATUS_INVALID_IMAGE_FORMAT;

    *size = headers->OptionalHeader.SizeOfImage;
    *baseAddress = reinterpret_cast<ULONG64>(&__ImageBase);

    return STATUS_SUCCESS;
}

PVOID Utils::AllocatePageAligned(const SIZE_T size)
{
    if (size < PAGE_SIZE)
        KeBugCheck(HAL_MEMORY_ALLOCATION);

    const PVOID allocated = AllocateContiguousMemory(size);
    if (!allocated)
        KeBugCheck(HAL_MEMORY_ALLOCATION);

    RtlZeroMemory(allocated, size);

    return allocated;
}

PEPROCESS Utils::GetNextProcess(PEPROCESS input)
{
    const PLIST_ENTRY currentListEntry = reinterpret_cast<PLIST_ENTRY>(reinterpret_cast<ULONG64>(input) + Global::Offsets::ActiveProcessLinks);
    PLIST_ENTRY nextListEntry = currentListEntry->Flink;
    return reinterpret_cast<PEPROCESS>(reinterpret_cast<ULONG64>(nextListEntry) - Global::Offsets::ActiveProcessLinks);
}

PEPROCESS Utils::FindProcess(const HANDLE processId)
{
    for (PEPROCESS current = PsInitialSystemProcess; current != nullptr; current = GetNextProcess(current))
    {
        const HANDLE currentId = PsGetProcessId(current);
        if (currentId == processId)
            return current;
    }

    return nullptr;
}

bool Utils::CheckMSR(UINT32 msr)
{
    if (((msr > 0) && (msr < 0x00001FFF)) || ((msr > 0xC0000000) && (msr < 0xC0002FFF)) || (msr > 0xC0010000) && (msr < 0xC0011FFF))
        return true;

    return false;
}

bool Utils::ValidUsermodeAddress(const ULONG64 address)
{
    if (address < 0x1000)
        return false;

    if (address > 0x7FFFFFFFFFFF)
        return false;

    return true;
}

void Utils::ReferenceObject(PVOID object)
{
    _InterlockedIncrement64(static_cast<volatile LONG64*>(object) - 6);
}

void Utils::DereferenceObject(PVOID object)
{
    _InterlockedExchangeAdd64(static_cast<volatile LONG64*>(object) - 6, -1);
}

PVOID Utils::GetPreallocatedPool(SIZE_T size)
{
    if (size > PreallocatedPoolSize)
        KeBugCheck(BAD_EXHANDLE);

    static SIZE_T currentPoolIndex = 0;
    if (currentPoolIndex >= ARRAYSIZE(Global::PreallocatedPools))
        KeBugCheck(BAD_EXHANDLE);

    const PVOID pool = Global::PreallocatedPools[currentPoolIndex];
    currentPoolIndex++;

    RtlZeroMemory(pool, size);

    return pool;
}
