#pragma once

namespace Memory
{
#define PFN_TO_PAGE(pfn) (pfn << PAGE_SHIFT)
#define PAGE_TO_PFN(pfn) (pfn >> PAGE_SHIFT)

#pragma warning(push)
#pragma warning(disable : 4201) // nonstandard extension used: nameless struct/union

#pragma pack(push, 1)
    typedef union CR3_
    {
        ULONG64 Value;
        struct
        {
            ULONG64 Ignored1 : 3;
            ULONG64 WriteThrough : 1;
            ULONG64 CacheDisable : 1;
            ULONG64 Ignored2 : 7;
            ULONG64 Pml4 : 40;
            ULONG64 Reserved : 12;
        };
    } PTE_CR3;

    typedef union VIRT_ADDR_
    {
        ULONG64 Value;
        void* Pointer;
        struct
        {
            ULONG64 Offset : 12;
            ULONG64 PtIndex : 9;
            ULONG64 PdIndex : 9;
            ULONG64 PdptIndex : 9;
            ULONG64 Pml4Index : 9;
            ULONG64 Reserved : 16;
        };
    } VIRTUAL_ADDRESS;

    typedef union PML4E_
    {
        ULONG64 Value;
        struct
        {
            ULONG64 Present : 1;
            ULONG64 Rw : 1;
            ULONG64 User : 1;
            ULONG64 WriteThrough : 1;
            ULONG64 CacheDisable : 1;
            ULONG64 Accessed : 1;
            ULONG64 Ignored1 : 1;
            ULONG64 Reserved1 : 1;
            ULONG64 Ignored2 : 4;
            ULONG64 Pdpt : 40;
            ULONG64 Ignored3 : 11;
            ULONG64 Xd : 1;
        };
    } PML4E;

    typedef union PDPTE_
    {
        ULONG64 Value;
        struct
        {
            ULONG64 Present : 1;
            ULONG64 Rw : 1;
            ULONG64 User : 1;
            ULONG64 WriteThrough : 1;
            ULONG64 CacheDisable : 1;
            ULONG64 Accessed : 1;
            ULONG64 Dirty : 1;
            ULONG64 PageSize : 1;
            ULONG64 Ignored2 : 4;
            ULONG64 Pd : 40;
            ULONG64 Ignored3 : 11;
            ULONG64 Xd : 1;
        };
    } PDPTE;

    typedef union PDE_
    {
        ULONG64 Value;
        struct
        {
            ULONG64 Present : 1;
            ULONG64 Rw : 1;
            ULONG64 User : 1;
            ULONG64 WriteThrough : 1;
            ULONG64 CacheDisable : 1;
            ULONG64 Accessed : 1;
            ULONG64 Dirty : 1;
            ULONG64 PageSize : 1;
            ULONG64 Ignored2 : 4;
            ULONG64 Pt : 40;
            ULONG64 Ignored3 : 11;
            ULONG64 Xd : 1;
        };
    } PDE;

    typedef union PTE_
    {
        ULONG64 Value;
        VIRTUAL_ADDRESS VirtualAddress;
        struct
        {
            ULONG64 Present : 1;
            ULONG64 Rw : 1;
            ULONG64 User : 1;
            ULONG64 WriteThrough : 1;
            ULONG64 CacheDisable : 1;
            ULONG64 Accessed : 1;
            ULONG64 Dirty : 1;
            ULONG64 Pat : 1;
            ULONG64 Global : 1;
            ULONG64 Ignored1 : 3;
            ULONG64 PageFrame : 40;
            ULONG64 Ignored3 : 11;
            ULONG64 Xd : 1;
        };
    } PTE;

    typedef struct _MMPTE_HARDWARE
    {
        struct /* bitfield */
        {
            /* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
            /* 0x0000 */ unsigned __int64 Dirty1 : 1; /* bit position: 1 */
            /* 0x0000 */ unsigned __int64 Owner : 1; /* bit position: 2 */
            /* 0x0000 */ unsigned __int64 WriteThrough : 1; /* bit position: 3 */
            /* 0x0000 */ unsigned __int64 CacheDisable : 1; /* bit position: 4 */
            /* 0x0000 */ unsigned __int64 Accessed : 1; /* bit position: 5 */
            /* 0x0000 */ unsigned __int64 Dirty : 1; /* bit position: 6 */
            /* 0x0000 */ unsigned __int64 LargePage : 1; /* bit position: 7 */
            /* 0x0000 */ unsigned __int64 Global : 1; /* bit position: 8 */
            /* 0x0000 */ unsigned __int64 CopyOnWrite : 1; /* bit position: 9 */
            /* 0x0000 */ unsigned __int64 Unused : 1; /* bit position: 10 */
            /* 0x0000 */ unsigned __int64 Write : 1; /* bit position: 11 */
            /* 0x0000 */ unsigned __int64 PageFrameNumber : 36; /* bit position: 12 */
            /* 0x0000 */ unsigned __int64 ReservedForHardware : 4; /* bit position: 48 */
            /* 0x0000 */ unsigned __int64 ReservedForSoftware : 4; /* bit position: 52 */
            /* 0x0000 */ unsigned __int64 WsleAge : 4; /* bit position: 56 */
            /* 0x0000 */ unsigned __int64 WsleProtection : 3; /* bit position: 60 */
            /* 0x0000 */ unsigned __int64 NoExecute : 1; /* bit position: 63 */
        }; /* bitfield */
    } MMPTE_HARDWARE, * PMMPTE_HARDWARE; /* size: 0x0008 */

    typedef struct _MMPTE_PROTOTYPE
    {
        struct /* bitfield */
        {
            /* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
            /* 0x0000 */ unsigned __int64 DemandFillProto : 1; /* bit position: 1 */
            /* 0x0000 */ unsigned __int64 HiberVerifyConverted : 1; /* bit position: 2 */
            /* 0x0000 */ unsigned __int64 ReadOnly : 1; /* bit position: 3 */
            /* 0x0000 */ unsigned __int64 SwizzleBit : 1; /* bit position: 4 */
            /* 0x0000 */ unsigned __int64 Protection : 5; /* bit position: 5 */
            /* 0x0000 */ unsigned __int64 Prototype : 1; /* bit position: 10 */
            /* 0x0000 */ unsigned __int64 Combined : 1; /* bit position: 11 */
            /* 0x0000 */ unsigned __int64 Unused1 : 4; /* bit position: 12 */
            /* 0x0000 */ __int64 ProtoAddress : 48; /* bit position: 16 */
        }; /* bitfield */
    } MMPTE_PROTOTYPE, * PMMPTE_PROTOTYPE; /* size: 0x0008 */

    typedef struct _MMPTE_SOFTWARE
    {
        struct /* bitfield */
        {
            /* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
            /* 0x0000 */ unsigned __int64 PageFileReserved : 1; /* bit position: 1 */
            /* 0x0000 */ unsigned __int64 PageFileAllocated : 1; /* bit position: 2 */
            /* 0x0000 */ unsigned __int64 ColdPage : 1; /* bit position: 3 */
            /* 0x0000 */ unsigned __int64 SwizzleBit : 1; /* bit position: 4 */
            /* 0x0000 */ unsigned __int64 Protection : 5; /* bit position: 5 */
            /* 0x0000 */ unsigned __int64 Prototype : 1; /* bit position: 10 */
            /* 0x0000 */ unsigned __int64 Transition : 1; /* bit position: 11 */
            /* 0x0000 */ unsigned __int64 PageFileLow : 4; /* bit position: 12 */
            /* 0x0000 */ unsigned __int64 UsedPageTableEntries : 10; /* bit position: 16 */
            /* 0x0000 */ unsigned __int64 ShadowStack : 1; /* bit position: 26 */
            /* 0x0000 */ unsigned __int64 Unused : 5; /* bit position: 27 */
            /* 0x0000 */ unsigned __int64 PageFileHigh : 32; /* bit position: 32 */
        }; /* bitfield */
    } MMPTE_SOFTWARE, * PMMPTE_SOFTWARE; /* size: 0x0008 */

    typedef struct _MMPTE_TIMESTAMP
    {
        struct /* bitfield */
        {
            /* 0x0000 */ unsigned __int64 MustBeZero : 1; /* bit position: 0 */
            /* 0x0000 */ unsigned __int64 Unused : 3; /* bit position: 1 */
            /* 0x0000 */ unsigned __int64 SwizzleBit : 1; /* bit position: 4 */
            /* 0x0000 */ unsigned __int64 Protection : 5; /* bit position: 5 */
            /* 0x0000 */ unsigned __int64 Prototype : 1; /* bit position: 10 */
            /* 0x0000 */ unsigned __int64 Transition : 1; /* bit position: 11 */
            /* 0x0000 */ unsigned __int64 PageFileLow : 4; /* bit position: 12 */
            /* 0x0000 */ unsigned __int64 Reserved : 16; /* bit position: 16 */
            /* 0x0000 */ unsigned __int64 GlobalTimeStamp : 32; /* bit position: 32 */
        }; /* bitfield */
    } MMPTE_TIMESTAMP, * PMMPTE_TIMESTAMP; /* size: 0x0008 */

    typedef struct _MMPTE_TRANSITION
    {
        struct /* bitfield */
        {
            /* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
            /* 0x0000 */ unsigned __int64 Write : 1; /* bit position: 1 */
            /* 0x0000 */ unsigned __int64 Spare : 1; /* bit position: 2 */
            /* 0x0000 */ unsigned __int64 IoTracker : 1; /* bit position: 3 */
            /* 0x0000 */ unsigned __int64 SwizzleBit : 1; /* bit position: 4 */
            /* 0x0000 */ unsigned __int64 Protection : 5; /* bit position: 5 */
            /* 0x0000 */ unsigned __int64 Prototype : 1; /* bit position: 10 */
            /* 0x0000 */ unsigned __int64 Transition : 1; /* bit position: 11 */
            /* 0x0000 */ unsigned __int64 PageFrameNumber : 36; /* bit position: 12 */
            /* 0x0000 */ unsigned __int64 Unused : 16; /* bit position: 48 */
        }; /* bitfield */
    } MMPTE_TRANSITION, * PMMPTE_TRANSITION; /* size: 0x0008 */

    typedef struct _MMPTE_SUBSECTION
    {
        struct /* bitfield */
        {
            /* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
            /* 0x0000 */ unsigned __int64 Unused0 : 3; /* bit position: 1 */
            /* 0x0000 */ unsigned __int64 SwizzleBit : 1; /* bit position: 4 */
            /* 0x0000 */ unsigned __int64 Protection : 5; /* bit position: 5 */
            /* 0x0000 */ unsigned __int64 Prototype : 1; /* bit position: 10 */
            /* 0x0000 */ unsigned __int64 ColdPage : 1; /* bit position: 11 */
            /* 0x0000 */ unsigned __int64 Unused1 : 3; /* bit position: 12 */
            /* 0x0000 */ unsigned __int64 ExecutePrivilege : 1; /* bit position: 15 */
            /* 0x0000 */ __int64 SubsectionAddress : 48; /* bit position: 16 */
        }; /* bitfield */
    } MMPTE_SUBSECTION, * PMMPTE_SUBSECTION; /* size: 0x0008 */

    typedef struct _MMPTE_LIST
    {
        struct /* bitfield */
        {
            /* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
            /* 0x0000 */ unsigned __int64 OneEntry : 1; /* bit position: 1 */
            /* 0x0000 */ unsigned __int64 filler0 : 2; /* bit position: 2 */
            /* 0x0000 */ unsigned __int64 SwizzleBit : 1; /* bit position: 4 */
            /* 0x0000 */ unsigned __int64 Protection : 5; /* bit position: 5 */
            /* 0x0000 */ unsigned __int64 Prototype : 1; /* bit position: 10 */
            /* 0x0000 */ unsigned __int64 Transition : 1; /* bit position: 11 */
            /* 0x0000 */ unsigned __int64 filler1 : 16; /* bit position: 12 */
            /* 0x0000 */ unsigned __int64 NextEntry : 36; /* bit position: 28 */
        }; /* bitfield */
    } MMPTE_LIST, * PMMPTE_LIST; /* size: 0x0008 */

    typedef struct _MMPTE
    {
        union
        {
            union
            {
                /* 0x0000 */ unsigned __int64 Long;
                /* 0x0000 */ volatile unsigned __int64 VolatileLong;
                /* 0x0000 */ struct _MMPTE_HARDWARE Hard;
                /* 0x0000 */ struct _MMPTE_PROTOTYPE Proto;
                /* 0x0000 */ struct _MMPTE_SOFTWARE Soft;
                /* 0x0000 */ struct _MMPTE_TIMESTAMP TimeStamp;
                /* 0x0000 */ struct _MMPTE_TRANSITION Trans;
                /* 0x0000 */ struct _MMPTE_SUBSECTION Subsect;
                /* 0x0000 */ struct _MMPTE_LIST List;
            }; /* size: 0x0008 */
        } /* size: 0x0008 */ u;
    } MMPTE, * PMMPTE; /* size: 0x0008 */
#pragma pack(pop)

    typedef struct _PAGE_INFO
    {
        PVOID VirtualAddress;
        PTE* PageEntry;
        ULONG64 PreviousPageFrame;
        PVOID CopyBuffer;
    } PAGE_INFO;

    ULONG64 GetDirectoryBase(PEPROCESS process);
    ULONG64 VirtualToPhysical(ULONG64 virtualAddress);
    ULONG64 PhysicalToVirtual(ULONG64 physicalAddress);
    ULONG64 ResolveProcessPhysicalAddress(UINT32 pageIndex, ULONG64 directoryBase, ULONG64 virtualAddress);
    PTE* GetPte(ULONG64 address);
    bool PreparePage(PAGE_INFO* targetPage);
    bool PreparePages();
    PVOID OverwritePage(UINT32 pageIndex, ULONG64 physicalAddress);
    void RestorePage(UINT32 pageIndex);
    void ReadPhysicalAddress(UINT32 pageIndex, ULONG64 targetAddress, PVOID buffer, SIZE_T size);
    void WritePhysicalAddress(UINT32 pageIndex, ULONG64 targetAddress, PVOID buffer, SIZE_T size);
    NTSTATUS ReadProcessMemory(UINT32 pageIndex, ULONG64 directoryBase, ULONG64 address, PVOID buffer, SIZE_T size, SIZE_T* bytesRead);
    NTSTATUS WriteProcessMemory(UINT32 pageIndex, ULONG64 directoryBase, ULONG64 address, PVOID buffer, SIZE_T size, SIZE_T* bytesWritten);
    NTSTATUS CopyProcessMemory(UINT32 pageIndex, ULONG64 sourceDirectoryBase, ULONG64 sourceAddress, ULONG64 targetDirectoryBase, ULONG64 targetAddress, SIZE_T bufferSize);
}
