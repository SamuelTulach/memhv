#pragma once

namespace SVM
{
    EXTERN_C void _sgdt(PVOID descriptor);
    EXTERN_C void NTAPI LaunchVM(PVOID hostRsp);

    NTSTATUS CheckSupport();
    void BuildNestedPageTables(PSHARED_VIRTUAL_PROCESSOR_DATA sharedData);
    void ProtectSelf(PSHARED_VIRTUAL_PROCESSOR_DATA sharedData);
    void BuildPermissionMap(PVOID permissionMap);
    bool CheckAndSetInstalled();
    UINT16 GetSegmentAccessRights(UINT16 segmentSelector, ULONG_PTR gdtBase);
    void PrepareForVirtualization(PVIRTUAL_PROCESSOR_DATA vpData, PSHARED_VIRTUAL_PROCESSOR_DATA sharedVpData, const PCONTEXT contextRecord);
    NTSTATUS VirtualizeProcessor(PVOID context);
    void VirtualizeAllProcessors(PVOID);
}