#pragma once

namespace SVM
{
    typedef struct _SEGMENT_DESCRIPTOR
    {
        union
        {
            UINT64 AsUInt64;
            struct
            {
                UINT16 LimitLow;        // [0:15]
                UINT16 BaseLow;         // [16:31]
                UINT32 BaseMiddle : 8;  // [32:39]
                UINT32 Type : 4;        // [40:43]
                UINT32 System : 1;      // [44]
                UINT32 Dpl : 2;         // [45:46]
                UINT32 Present : 1;     // [47]
                UINT32 LimitHigh : 4;   // [48:51]
                UINT32 Avl : 1;         // [52]
                UINT32 LongMode : 1;    // [53]
                UINT32 DefaultBit : 1;  // [54]
                UINT32 Granularity : 1; // [55]
                UINT32 BaseHigh : 8;    // [56:63]
            } Fields;
        };
    } SEGMENT_DESCRIPTOR, * PSEGMENT_DESCRIPTOR;
    static_assert(sizeof(SEGMENT_DESCRIPTOR) == 8, "SEGMENT_DESCRIPTOR size mismatch");

    typedef struct _SEGMENT_ATTRIBUTE
    {
        union
        {
            UINT16 AsUInt16;
            struct
            {
                UINT16 Type : 4;        // [0:3]
                UINT16 System : 1;      // [4]
                UINT16 Dpl : 2;         // [5:6]
                UINT16 Present : 1;     // [7]
                UINT16 Avl : 1;         // [8]
                UINT16 LongMode : 1;    // [9]
                UINT16 DefaultBit : 1;  // [10]
                UINT16 Granularity : 1; // [11]
                UINT16 Reserved1 : 4;   // [12:15]
            } Fields;
        };
    } SEGMENT_ATTRIBUTE, * PSEGMENT_ATTRIBUTE;
    static_assert(sizeof(SEGMENT_ATTRIBUTE) == 2, "SEGMENT_ATTRIBUTE size mismatch");

    typedef struct _SHARED_VIRTUAL_PROCESSOR_DATA
    {
        PVOID MsrPermissionsMap;
        DECLSPEC_ALIGN(PAGE_SIZE) PML4E_64* Pml4Entries;
        DECLSPEC_ALIGN(PAGE_SIZE) PDPTE_1GB(*PdpEntries)[512];
    } SHARED_VIRTUAL_PROCESSOR_DATA, * PSHARED_VIRTUAL_PROCESSOR_DATA;

    typedef struct _VIRTUAL_PROCESSOR_DATA
    {
        union
        {
            DECLSPEC_ALIGN(PAGE_SIZE) UINT8 HostStackLimit[KERNEL_STACK_SIZE];
            struct
            {
                UINT8 StackContents[KERNEL_STACK_SIZE - (sizeof(PVOID) * 6) - sizeof(KTRAP_FRAME)];
                KTRAP_FRAME TrapFrame;
                UINT64 GuestVmcbPa;
                UINT64 HostVmcbPa;
                struct _VIRTUAL_PROCESSOR_DATA* Self;
                PSHARED_VIRTUAL_PROCESSOR_DATA SharedVpData;
                UINT64 ProcessorIndex;
                UINT64 Reserved1;
            } HostStackLayout;
        };

        DECLSPEC_ALIGN(PAGE_SIZE) VMCB GuestVmcb;
        DECLSPEC_ALIGN(PAGE_SIZE) VMCB HostVmcb;
        DECLSPEC_ALIGN(PAGE_SIZE) UINT8 HostStateArea[PAGE_SIZE];
    } VIRTUAL_PROCESSOR_DATA, * PVIRTUAL_PROCESSOR_DATA;
    static_assert(sizeof(VIRTUAL_PROCESSOR_DATA) == KERNEL_STACK_SIZE + PAGE_SIZE * 3, "VIRTUAL_PROCESSOR_DATA size mismatch");

    typedef struct _GUEST_REGISTERS
    {
        UINT64 R15;
        UINT64 R14;
        UINT64 R13;
        UINT64 R12;
        UINT64 R11;
        UINT64 R10;
        UINT64 R9;
        UINT64 R8;
        UINT64 Rdi;
        UINT64 Rsi;
        UINT64 Rbp;
        UINT64 Rsp;
        UINT64 Rbx;
        UINT64 Rdx;
        UINT64 Rcx;
        UINT64 Rax;
    } GUEST_REGISTERS, * PGUEST_REGISTERS;

    typedef struct _GUEST_CONTEXT
    {
        PGUEST_REGISTERS VpRegs;
        BOOLEAN ExitVm;
    } GUEST_CONTEXT, * PGUEST_CONTEXT;

    typedef struct _EVENTINJ
    {
        union
        {
            UINT64 AsUInt64;
            struct
            {
                UINT64 Vector : 8;          // [0:7]
                UINT64 Type : 3;            // [8:10]
                UINT64 ErrorCodeValid : 1;  // [11]
                UINT64 Reserved1 : 19;      // [12:30]
                UINT64 Valid : 1;           // [31]
                UINT64 ErrorCode : 32;      // [32:63]
            } Fields;
        };
    } EVENTINJ, * PEVENTINJ;
    static_assert(sizeof(EVENTINJ) == 8, "EVENTINJ size mismatch");

    typedef struct _NPF_EXITINFO1
    {
        union
        {
            UINT64 AsUInt64;
            struct
            {
                UINT64 Valid : 1;                   // [0]
                UINT64 Write : 1;                   // [1]
                UINT64 User : 1;                    // [2]
                UINT64 Reserved : 1;                // [3]
                UINT64 Execute : 1;                 // [4]
                UINT64 Reserved2 : 27;              // [5:31]
                UINT64 GuestPhysicalAddress : 1;    // [32]
                UINT64 GuestPageTables : 1;         // [33]
            } Fields;
        };
    } NPF_EXITINFO1, * PNPF_EXITINFO1;
    static_assert(sizeof(NPF_EXITINFO1) == 8, "NPF_EXITINFO1 size mismatch");
}
