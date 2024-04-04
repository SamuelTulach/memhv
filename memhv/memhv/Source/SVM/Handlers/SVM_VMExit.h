#pragma once

namespace SVM
{
    void InjectGeneralProtectionException(PVIRTUAL_PROCESSOR_DATA vpData);

    bool IsInUserland(PVIRTUAL_PROCESSOR_DATA vpData);

    void HandleGenericSVM(PVIRTUAL_PROCESSOR_DATA vpData, PGUEST_CONTEXT guestContext);
    void HandleVMCall(PVIRTUAL_PROCESSOR_DATA vpData, PGUEST_CONTEXT guestContext);
    void HandleMSRAccess(PVIRTUAL_PROCESSOR_DATA vpData, PGUEST_CONTEXT guestContext);

    EXTERN_C bool HandleExit(PVIRTUAL_PROCESSOR_DATA vpData, PGUEST_REGISTERS guestRegisters);
}
