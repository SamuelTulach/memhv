#include "../../Global.h"

void SVM::HandleGenericSVM(const PVIRTUAL_PROCESSOR_DATA vpData, const PGUEST_CONTEXT guestContext)
{
    UNREFERENCED_PARAMETER(guestContext);

    InjectGeneralProtectionException(vpData);
}