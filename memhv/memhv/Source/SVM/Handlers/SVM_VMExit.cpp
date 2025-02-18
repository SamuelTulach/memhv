#include "../../Global.h"

void SVM::InjectGeneralProtectionException(const PVIRTUAL_PROCESSOR_DATA vpData)
{
    EVENTINJ event;
    event.AsUInt64 = 0;
    event.Fields.Vector = EXCEPTION_VECTOR_GENERAL_PROTECTION_FAULT;
    event.Fields.Type = INTERRUPT_TYPE_HARDWARE_EXCEPTION;
    event.Fields.ErrorCodeValid = 1;
    event.Fields.Valid = 1;
    vpData->GuestVmcb.ControlArea.EventInj = event.AsUInt64;
}

/*void SVM::InjectUndefinedOpcodeException(const PVIRTUAL_PROCESSOR_DATA vpData)
{
    EVENTINJ event;
    event.AsUInt64 = 0;
    event.Fields.Vector = EXCEPTION_VECTOR_UNDEFINED_OPCODE;
    event.Fields.Type = INTERRUPT_TYPE_HARDWARE_EXCEPTION;
    event.Fields.ErrorCodeValid = 0;
    event.Fields.Present = 1;
    vpData->GuestVmcb.ControlArea.EventInj = event.AsUInt64;
}

void SVM::InjectPageFaultException(const PVIRTUAL_PROCESSOR_DATA vpData, const ULONG64 address)
{
    // https://wiki.osdev.org/Exceptions#Page_Fault
    EVENTINJ event;
    event.AsUInt64 = 0;
    event.Fields.Vector = EXCEPTION_VECTOR_PAGE_FAULT;
    event.Fields.Type = INTERRUPT_TYPE_HARDWARE_EXCEPTION;
    event.Fields.ErrorCodeValid = 1;
    event.Fields.Present = 1;

    PAGE_FAULT_EXCEPTION errorCode;
    errorCode.AsUInt = 0;
    errorCode.UserModeAccess = 1;

    event.Fields.ErrorCode = errorCode.AsUInt;

    vpData->GuestVmcb.StateSaveArea.Cr2 = address;

    vpData->GuestVmcb.ControlArea.EventInj = event.AsUInt64;
}*/

bool SVM::IsInUserland(PVIRTUAL_PROCESSOR_DATA vpData)
{
    return vpData->GuestVmcb.StateSaveArea.Cpl == 3 && vpData->GuestVmcb.ControlArea.NRip < 0x7FFFFFFEFFFF;
}

EXTERN_C bool SVM::HandleExit(PVIRTUAL_PROCESSOR_DATA vpData, const PGUEST_REGISTERS guestRegisters)
{
    GUEST_CONTEXT guestContext;
    guestContext.VpRegs = guestRegisters;
    guestContext.ExitVm = false;

    __svm_vmload(vpData->HostStackLayout.HostVmcbPa);

    guestRegisters->Rax = vpData->GuestVmcb.StateSaveArea.Rax;

    vpData->HostStackLayout.TrapFrame.Rsp = vpData->GuestVmcb.StateSaveArea.Rsp;
    vpData->HostStackLayout.TrapFrame.Rip = vpData->GuestVmcb.ControlArea.NRip;

    switch (vpData->GuestVmcb.ControlArea.ExitCode)
    {
    case VMEXIT_MSR:
        HandleMSRAccess(vpData, &guestContext);
        break;
    case VMEXIT_VMRUN:
    case VMEXIT_VMLOAD:
    case VMEXIT_VMSAVE:
        HandleGenericSVM(vpData, &guestContext);
        break;
    case VMEXIT_VMMCALL:
        HandleVMCall(vpData, &guestContext);
        break;
    default:
        KeBugCheckEx(INVALID_DRIVER_HANDLE, IsInUserland(vpData), vpData->GuestVmcb.StateSaveArea.Rip, vpData->GuestVmcb.ControlArea.ExitCode, vpData->GuestVmcb.ControlArea.ExitInfo1);
    }

    if (guestContext.ExitVm)
    {
        guestContext.VpRegs->Rax = reinterpret_cast<UINT64>(vpData) & MAXUINT32;
        guestContext.VpRegs->Rbx = vpData->GuestVmcb.ControlArea.NRip;
        guestContext.VpRegs->Rcx = vpData->GuestVmcb.StateSaveArea.Rsp;
        guestContext.VpRegs->Rdx = reinterpret_cast<UINT64>(vpData) >> 32;

        __svm_vmload(MmGetPhysicalAddress(&vpData->GuestVmcb).QuadPart);

        _disable();
        __svm_stgi();

        __writemsr(IA32_MSR_EFER, __readmsr(IA32_MSR_EFER) & ~EFER_SVME);
        __writeeflags(vpData->GuestVmcb.StateSaveArea.Rflags);

        return true;
    }

    vpData->GuestVmcb.StateSaveArea.Rax = guestContext.VpRegs->Rax;

    return false;
}