#include "Global.h"

EXTERN_C NTSTATUS Entry()
{
    if (!Memory::PreparePages())
        return STATUS_INSUFFICIENT_RESOURCES;

    NTSTATUS status = SVM::CheckSupport();
    if (!NT_SUCCESS(status))
        return status;

    HANDLE threadHandle;
    status = PsCreateSystemThread(&threadHandle, THREAD_ALL_ACCESS, nullptr, nullptr, nullptr, SVM::VirtualizeAllProcessors, nullptr);
    if (!NT_SUCCESS(status))
        return status;

    return status;
}