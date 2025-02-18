#pragma once

#define NESTED_MODE true

#include <ntifs.h>
#include <intrin.h>
#include <ntdef.h>
#include <minwindef.h>
#include <stddef.h>
#include <ntimage.h>

#include "Utils.h"
#include "Shared.h"

#include "Memory/Physical.h"

#include "SVM/Defines/SVM_Platform.h"
#include "SVM/Defines/SVM_NestedPaging.h"
#include "SVM/Defines/SVM_ControlArea.h"
#include "SVM/Defines/SVM_ProcessorData.h"
#include "SVM/SVM.h"
#include "SVM/Handlers/SVM_VMExit.h"

namespace Global
{
    inline PVOID BlankPage = nullptr;
    inline PVOID PreallocatedPools[32];

    namespace Offsets
    {
        inline ULONG64 ActiveProcessLinks = 0x448;
    }
}