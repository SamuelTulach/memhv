#include "../../Global.h"

FORCEINLINE bool CheckRange(const ULONG64 input, const ULONG64 rangeStart, const ULONG64 rangeEnd)
{
    if (input >= rangeStart && input <= rangeEnd)
        return true;

    return false;
}

void SVM::HandleMSRAccess(const PVIRTUAL_PROCESSOR_DATA vpData, const PGUEST_CONTEXT guestContext)
{
    const UINT32 msr = guestContext->VpRegs->Rcx & MAXUINT32;
    const BOOLEAN writeAccess = (vpData->GuestVmcb.ControlArea.ExitInfo1 != 0);

#if !NESTED_MODE
    if (!Utils::CheckMSR(msr))
    {
        InjectGeneralProtectionException(vpData);
        return;
    }
#endif

    ULARGE_INTEGER value;
    if (msr == IA32_MSR_EFER)
    {
        if (writeAccess)
        {
            value.LowPart = guestContext->VpRegs->Rax & MAXUINT32;
            value.HighPart = guestContext->VpRegs->Rdx & MAXUINT32;
            value.QuadPart |= EFER_SVME;

            vpData->GuestVmcb.StateSaveArea.Efer = value.QuadPart;
        }
        else
        {
            value.QuadPart = __readmsr(msr);
            value.QuadPart &= ~EFER_SVME;
            guestContext->VpRegs->Rax = value.LowPart;
            guestContext->VpRegs->Rdx = value.HighPart;
        }
    }
    else
    {
        if (writeAccess)
        {
            value.LowPart = guestContext->VpRegs->Rax & MAXUINT32;
            value.HighPart = guestContext->VpRegs->Rdx & MAXUINT32;
            __writemsr(msr, value.QuadPart);
        }
        else
        {
            value.QuadPart = __readmsr(msr);
            guestContext->VpRegs->Rax = value.LowPart;
            guestContext->VpRegs->Rdx = value.HighPart;
        }
    }

    vpData->GuestVmcb.StateSaveArea.Rip = vpData->GuestVmcb.ControlArea.NRip;
}