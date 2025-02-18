#include "Global.h"

NTSTATUS GetOffsets()
{
    UNICODE_STRING stringPsGetProcessId;
    RtlInitUnicodeString(&stringPsGetProcessId, L"PsGetProcessId");

    BYTE* routinePsGetProcessId = static_cast<BYTE*>(MmGetSystemRoutineAddress(&stringPsGetProcessId));
    if (!routinePsGetProcessId)
        return STATUS_NOT_FOUND;

    // PsGetProcessId proc near
    // mov rax, [rcx + 1D0h]
    // 48 8B 81 (D0 01 00 00)
    // VOID* UniqueProcessId;
    // struct _LIST_ENTRY ActiveProcessLinks;
    Global::Offsets::ActiveProcessLinks = *reinterpret_cast<UINT32*>(routinePsGetProcessId + 3) + 8;

    return STATUS_SUCCESS;
}

NTSTATUS PreallocatePools()
{
    for (UINT32 i = 0; i < ARRAYSIZE(Global::PreallocatedPools); i++)
    {
        Global::PreallocatedPools[i] = Utils::AllocatePageAligned(Utils::PreallocatedPoolSize);
        if (!Global::PreallocatedPools[i])
            return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

EXTERN_C NTSTATUS Entry()
{
    if (!Memory::PreparePages())
        return STATUS_INSUFFICIENT_RESOURCES;

    NTSTATUS status = SVM::CheckSupport();
    if (!NT_SUCCESS(status))
        return status;

    status = GetOffsets();
    if (!NT_SUCCESS(status))
        return status;

    status = PreallocatePools();
    if (!NT_SUCCESS(status))
        return status;

    Global::BlankPage = Utils::AllocatePageAligned(PAGE_SIZE);
    if (!Global::BlankPage)
        return STATUS_INSUFFICIENT_RESOURCES;

    HANDLE threadHandle;
    status = PsCreateSystemThread(&threadHandle, THREAD_ALL_ACCESS, nullptr, nullptr, nullptr, SVM::VirtualizeAllProcessors, nullptr);
    if (!NT_SUCCESS(status))
        return status;

    return status;
}