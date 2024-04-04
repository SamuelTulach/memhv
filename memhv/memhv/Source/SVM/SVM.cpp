#include "../Global.h"

NTSTATUS SVM::CheckSupport()
{
    int registers[4];

    __cpuid(registers, CPUID_MAX_STANDARD_FN_NUMBER_AND_VENDOR_STRING);
    if ((registers[1] != 'htuA') ||
        (registers[3] != 'itne') ||
        (registers[2] != 'DMAc'))
        return STATUS_HV_INVALID_DEVICE_ID;

    __cpuid(registers, CPUID_PROCESSOR_AND_PROCESSOR_FEATURE_IDENTIFIERS_EX);
    if (!(registers[2] & CPUID_FN8000_0001_ECX_SVM))
        return STATUS_HV_CPUID_FEATURE_VALIDATION_ERROR;

    __cpuid(registers, CPUID_SVM_FEATURES);
    if (!(registers[3] & CPUID_FN8000_000A_EDX_NP))
        return STATUS_HV_CPUID_FEATURE_VALIDATION_ERROR;

    const ULONG64 vmcr = __readmsr(SVM_MSR_VM_CR);
    if (vmcr & SVM_VM_CR_SVMDIS)
        return STATUS_HV_INVALID_DEVICE_STATE;

    return STATUS_SUCCESS;
}

void SVM::BuildNestedPageTables(const PSHARED_VIRTUAL_PROCESSOR_DATA sharedData)
{
    const ULONG64 pdpBasePa = MmGetPhysicalAddress(&sharedData->PdpEntries).QuadPart;
    sharedData->Pml4Entries[0].Fields.PageFrameNumber = pdpBasePa >> PAGE_SHIFT;
    sharedData->Pml4Entries[0].Fields.Valid = 1;
    sharedData->Pml4Entries[0].Fields.Write = 1;
    sharedData->Pml4Entries[0].Fields.User = 1;

    for (ULONG64 pdpIndex = 0; pdpIndex < 512; pdpIndex++)
    {
        const ULONG64 pdeBasePa = MmGetPhysicalAddress(&sharedData->PdeEntries[pdpIndex][0]).QuadPart;
        sharedData->PdpEntries[pdpIndex].Fields.PageFrameNumber = pdeBasePa >> PAGE_SHIFT;
        sharedData->PdpEntries[pdpIndex].Fields.Valid = 1;
        sharedData->PdpEntries[pdpIndex].Fields.Write = 1;
        sharedData->PdpEntries[pdpIndex].Fields.User = 1;

        for (ULONG64 pdIndex = 0; pdIndex < 512; pdIndex++)
        {
            constexpr ULONG64 pml4Index = 0;
            const ULONG64 translationPa = ((pml4Index * 512 + pdpIndex) * 512) + pdIndex;
            sharedData->PdeEntries[pdpIndex][pdIndex].Fields.PageFrameNumber = translationPa;
            sharedData->PdeEntries[pdpIndex][pdIndex].Fields.Valid = 1;
            sharedData->PdeEntries[pdpIndex][pdIndex].Fields.Write = 1;
            sharedData->PdeEntries[pdpIndex][pdIndex].Fields.User = 1;
            sharedData->PdeEntries[pdpIndex][pdIndex].Fields.LargePage = 1;
        }
    }
}

void SVM::BuildPermissionMap(const PVOID permissionMap)
{
    static const UINT32 BITS_PER_MSR = 2;
    static const UINT32 SECOND_MSR_RANGE_BASE = 0xc0000000;
    static const UINT32 SECOND_MSRPM_OFFSET = 0x800 * CHAR_BIT;

    RTL_BITMAP bitmapHeader;
    RtlInitializeBitMap(&bitmapHeader, static_cast<PULONG>(permissionMap), SVM_MSR_PERMISSIONS_MAP_SIZE * CHAR_BIT);
    RtlClearAllBits(&bitmapHeader);

    const ULONG offsetFrom2ndBase = (IA32_MSR_EFER - SECOND_MSR_RANGE_BASE) * BITS_PER_MSR;
    const ULONG offset = SECOND_MSRPM_OFFSET + offsetFrom2ndBase;

    RtlSetBits(&bitmapHeader, offset + 1, 1);
}

static bool VirtualizedProcessors[64] = { 0 };
bool SVM::CheckAndSetInstalled()
{
    const ULONG processorIndex = KeGetCurrentProcessorNumber();
    if (VirtualizedProcessors[processorIndex])
        return true;

    VirtualizedProcessors[processorIndex] = true;
    return false;
}

UINT16 SVM::GetSegmentAccessRights(const UINT16 segmentSelector, const ULONG_PTR gdtBase)
{
    const PSEGMENT_DESCRIPTOR descriptor = reinterpret_cast<PSEGMENT_DESCRIPTOR>(gdtBase + (segmentSelector & ~RPL_MASK));

    SEGMENT_ATTRIBUTE attribute;
    attribute.Fields.Type = descriptor->Fields.Type;
    attribute.Fields.System = descriptor->Fields.System;
    attribute.Fields.Dpl = descriptor->Fields.Dpl;
    attribute.Fields.Present = descriptor->Fields.Present;
    attribute.Fields.Avl = descriptor->Fields.Avl;
    attribute.Fields.LongMode = descriptor->Fields.LongMode;
    attribute.Fields.DefaultBit = descriptor->Fields.DefaultBit;
    attribute.Fields.Granularity = descriptor->Fields.Granularity;
    attribute.Fields.Reserved1 = 0;

    return attribute.AsUInt16;
}

void SVM::PrepareForVirtualization(const PVIRTUAL_PROCESSOR_DATA vpData, const PSHARED_VIRTUAL_PROCESSOR_DATA sharedVpData, const PCONTEXT contextRecord)
{
    const ULONG processorIndex = KeGetCurrentProcessorNumber();
    vpData->HostStackLayout.ProcessorIndex = processorIndex;

    DESCRIPTOR_TABLE_REGISTER gdtr, idtr;
    _sgdt(&gdtr);
    __sidt(&idtr);

    const PHYSICAL_ADDRESS guestVmcbPa = MmGetPhysicalAddress(&vpData->GuestVmcb);
    const PHYSICAL_ADDRESS hostVmcbPa = MmGetPhysicalAddress(&vpData->HostVmcb);
    const PHYSICAL_ADDRESS hostStateAreaPa = MmGetPhysicalAddress(&vpData->HostStateArea);
    const PHYSICAL_ADDRESS pml4BasePa = MmGetPhysicalAddress(sharedVpData->Pml4Entries);
    const PHYSICAL_ADDRESS msrpmPa = MmGetPhysicalAddress(sharedVpData->MsrPermissionsMap);

    vpData->GuestVmcb.ControlArea.InterceptMisc2 |= SVM_INTERCEPT_MISC2_VMRUN;
    vpData->GuestVmcb.ControlArea.InterceptMisc2 |= SVM_INTERCEPT_MISC2_VMCALL;

    vpData->GuestVmcb.ControlArea.InterceptMisc1 |= SVM_INTERCEPT_MISC1_MSR_PROT;
    vpData->GuestVmcb.ControlArea.MsrpmBasePa = msrpmPa.QuadPart;

    vpData->GuestVmcb.ControlArea.GuestAsid = 1;

    vpData->GuestVmcb.ControlArea.NpEnable |= SVM_NP_ENABLE_NP_ENABLE;
    vpData->GuestVmcb.ControlArea.NCr3 = pml4BasePa.QuadPart;

    vpData->GuestVmcb.StateSaveArea.GdtrBase = gdtr.Base;
    vpData->GuestVmcb.StateSaveArea.GdtrLimit = gdtr.Limit;
    vpData->GuestVmcb.StateSaveArea.IdtrBase = idtr.Base;
    vpData->GuestVmcb.StateSaveArea.IdtrLimit = idtr.Limit;

    vpData->GuestVmcb.StateSaveArea.CsLimit = GetSegmentLimit(contextRecord->SegCs);
    vpData->GuestVmcb.StateSaveArea.DsLimit = GetSegmentLimit(contextRecord->SegDs);
    vpData->GuestVmcb.StateSaveArea.EsLimit = GetSegmentLimit(contextRecord->SegEs);
    vpData->GuestVmcb.StateSaveArea.SsLimit = GetSegmentLimit(contextRecord->SegSs);
    vpData->GuestVmcb.StateSaveArea.CsSelector = contextRecord->SegCs;
    vpData->GuestVmcb.StateSaveArea.DsSelector = contextRecord->SegDs;
    vpData->GuestVmcb.StateSaveArea.EsSelector = contextRecord->SegEs;
    vpData->GuestVmcb.StateSaveArea.SsSelector = contextRecord->SegSs;
    vpData->GuestVmcb.StateSaveArea.CsAttrib = GetSegmentAccessRights(contextRecord->SegCs, gdtr.Base);
    vpData->GuestVmcb.StateSaveArea.DsAttrib = GetSegmentAccessRights(contextRecord->SegDs, gdtr.Base);
    vpData->GuestVmcb.StateSaveArea.EsAttrib = GetSegmentAccessRights(contextRecord->SegEs, gdtr.Base);
    vpData->GuestVmcb.StateSaveArea.SsAttrib = GetSegmentAccessRights(contextRecord->SegSs, gdtr.Base);

    vpData->GuestVmcb.StateSaveArea.Efer = __readmsr(IA32_MSR_EFER);
    vpData->GuestVmcb.StateSaveArea.Cr0 = __readcr0();
    vpData->GuestVmcb.StateSaveArea.Cr2 = __readcr2();
    vpData->GuestVmcb.StateSaveArea.Cr3 = __readcr3();
    vpData->GuestVmcb.StateSaveArea.Cr4 = __readcr4();
    vpData->GuestVmcb.StateSaveArea.Rflags = contextRecord->EFlags;
    vpData->GuestVmcb.StateSaveArea.Rsp = contextRecord->Rsp;
    vpData->GuestVmcb.StateSaveArea.Rip = contextRecord->Rip;
    vpData->GuestVmcb.StateSaveArea.GPat = __readmsr(IA32_MSR_PAT);

    __svm_vmsave(guestVmcbPa.QuadPart);

    vpData->HostStackLayout.Reserved1 = MAXUINT64;
    vpData->HostStackLayout.SharedVpData = sharedVpData;
    vpData->HostStackLayout.Self = vpData;
    vpData->HostStackLayout.HostVmcbPa = hostVmcbPa.QuadPart;
    vpData->HostStackLayout.GuestVmcbPa = guestVmcbPa.QuadPart;

    __writemsr(SVM_MSR_VM_HSAVE_PA, hostStateAreaPa.QuadPart);

    __svm_vmsave(hostVmcbPa.QuadPart);
}

NTSTATUS SVM::VirtualizeProcessor(const PVOID context)
{
    const PCONTEXT contextRecord = static_cast<PCONTEXT>(ExAllocatePool(NonPagedPool, sizeof(CONTEXT)));
    if (!contextRecord)
        return STATUS_INSUFFICIENT_RESOURCES;

    const PVIRTUAL_PROCESSOR_DATA vpData = static_cast<PVIRTUAL_PROCESSOR_DATA>(Utils::AllocatePageAligned(sizeof(VIRTUAL_PROCESSOR_DATA)));
    if (!vpData)
        return STATUS_INSUFFICIENT_RESOURCES;

    RtlCaptureContext(contextRecord);

    if (!CheckAndSetInstalled())
    {
        const PSHARED_VIRTUAL_PROCESSOR_DATA sharedVpData = static_cast<PSHARED_VIRTUAL_PROCESSOR_DATA>(context);

        __writemsr(IA32_MSR_EFER, __readmsr(IA32_MSR_EFER) | EFER_SVME);

        PrepareForVirtualization(vpData, sharedVpData, contextRecord);

        LaunchVM(&vpData->HostStackLayout.GuestVmcbPa);

        KeBugCheck(MANUALLY_INITIATED_CRASH);
    }

    return STATUS_SUCCESS;
}

void SVM::VirtualizeAllProcessors(PVOID)
{
    const PSHARED_VIRTUAL_PROCESSOR_DATA sharedVpData = static_cast<PSHARED_VIRTUAL_PROCESSOR_DATA>(Utils::AllocatePageAligned(sizeof(SHARED_VIRTUAL_PROCESSOR_DATA)));
    if (!sharedVpData)
        return;

    sharedVpData->MsrPermissionsMap = Utils::AllocateContiguousMemory(SVM_MSR_PERMISSIONS_MAP_SIZE);
    if (!sharedVpData->MsrPermissionsMap)
        return;

    BuildNestedPageTables(sharedVpData);
    BuildPermissionMap(sharedVpData->MsrPermissionsMap);

    const NTSTATUS status = Utils::ExecuteOnEachProcessor(VirtualizeProcessor, sharedVpData);
    if (!NT_SUCCESS(status))
        KeBugCheck(MANUALLY_INITIATED_CRASH);
}