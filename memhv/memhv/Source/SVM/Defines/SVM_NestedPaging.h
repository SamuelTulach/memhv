#pragma once

namespace SVM
{
    typedef struct _APIC_BASE
    {
        union
        {
            UINT64 AsUInt64;
            struct
            {
                UINT64 Reserved1 : 8;           // [0:7]
                UINT64 BootstrapProcessor : 1;  // [8]
                UINT64 Reserved2 : 1;           // [9]
                UINT64 EnableX2ApicMode : 1;    // [10]
                UINT64 EnableXApicGlobal : 1;   // [11]
                UINT64 ApicBase : 24;           // [12:35]
            } Fields;
        };
    } APIC_BASE, * PAPIC_BASE;

    typedef struct _PML4_ENTRY_4KB
    {
        union
        {
            UINT64 AsUInt64;
            struct
            {
                UINT64 Valid : 1;               // [0]
                UINT64 Write : 1;               // [1]
                UINT64 User : 1;                // [2]
                UINT64 WriteThrough : 1;        // [3]
                UINT64 CacheDisable : 1;        // [4]
                UINT64 Accessed : 1;            // [5]
                UINT64 Reserved1 : 3;           // [6:8]
                UINT64 Avl : 3;                 // [9:11]
                UINT64 PageFrameNumber : 40;    // [12:51]
                UINT64 Reserved2 : 11;          // [52:62]
                UINT64 NoExecute : 1;           // [63]
            } Fields;
        };
    } PML4_ENTRY_4KB, * PPML4_ENTRY_4KB,
        PDP_ENTRY_4KB, * PPDP_ENTRY_4KB,
        PD_ENTRY_4KB, * PPD_ENTRY_4KB;
    static_assert(sizeof(PML4_ENTRY_4KB) == 8, "size mismatch");

    typedef struct _PT_ENTRY_4KB
    {
        union
        {
            UINT64 AsUInt64;
            struct
            {
                UINT64 Valid : 1;               // [0]
                UINT64 Write : 1;               // [1]
                UINT64 User : 1;                // [2]
                UINT64 WriteThrough : 1;        // [3]
                UINT64 CacheDisable : 1;        // [4]
                UINT64 Accessed : 1;            // [5]
                UINT64 Dirty : 1;               // [6]
                UINT64 Pat : 1;                 // [7]
                UINT64 Global : 1;              // [8]
                UINT64 Avl : 3;                 // [9:11]
                UINT64 PageFrameNumber : 40;    // [12:51]
                UINT64 Reserved1 : 11;          // [52:62]
                UINT64 NoExecute : 1;           // [63]
            } Fields;
        };
    } PT_ENTRY_4KB, * PPT_ENTRY_4KB;
    static_assert(sizeof(PT_ENTRY_4KB) == 8, "size mismatch");

    typedef struct _PML4_ENTRY_2MB
    {
        union
        {
            UINT64 AsUInt64;
            struct
            {
                UINT64 Valid : 1;               // [0]
                UINT64 Write : 1;               // [1]
                UINT64 User : 1;                // [2]
                UINT64 WriteThrough : 1;        // [3]
                UINT64 CacheDisable : 1;        // [4]
                UINT64 Accessed : 1;            // [5]
                UINT64 Reserved1 : 3;           // [6:8]
                UINT64 Avl : 3;                 // [9:11]
                UINT64 PageFrameNumber : 40;    // [12:51]
                UINT64 Reserved2 : 11;          // [52:62]
                UINT64 NoExecute : 1;           // [63]
            } Fields;
        };
    } PML4_ENTRY_2MB, * PPML4_ENTRY_2MB,
        PDP_ENTRY_2MB, * PPDP_ENTRY_2MB;
    static_assert(sizeof(PML4_ENTRY_2MB) == 8, "PML4_ENTRY_2MB size mismatch");

    typedef struct _PD_ENTRY_2MB
    {
        union
        {
            UINT64 AsUInt64;
            struct
            {
                UINT64 Valid : 1;               // [0]
                UINT64 Write : 1;               // [1]
                UINT64 User : 1;                // [2]
                UINT64 WriteThrough : 1;        // [3]
                UINT64 CacheDisable : 1;        // [4]
                UINT64 Accessed : 1;            // [5]
                UINT64 Dirty : 1;               // [6]
                UINT64 LargePage : 1;           // [7]
                UINT64 Global : 1;              // [8]
                UINT64 Avl : 3;                 // [9:11]
                UINT64 Pat : 1;                 // [12]
                UINT64 Reserved1 : 8;           // [13:20]
                UINT64 PageFrameNumber : 31;    // [21:51]
                UINT64 Reserved2 : 11;          // [52:62]
                UINT64 NoExecute : 1;           // [63]
            } Fields;
        };
    } PD_ENTRY_2MB, * PPD_ENTRY_2MB;
    static_assert(sizeof(PD_ENTRY_2MB) == 8, "PD_ENTRY_2MB size mismatch");

#include <pshpack1.h>
    typedef struct _DESCRIPTOR_TABLE_REGISTER
    {
        UINT16 Limit;
        ULONG_PTR Base;
    } DESCRIPTOR_TABLE_REGISTER, * PDESCRIPTOR_TABLE_REGISTER;
    static_assert(sizeof(DESCRIPTOR_TABLE_REGISTER) == 10,
        "DESCRIPTOR_TABLE_REGISTER size mismatch");
#include <poppack.h>
}