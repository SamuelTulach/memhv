#pragma once

namespace SVM
{
    typedef struct _VMCB_CONTROL_AREA
    {
        UINT16 InterceptCrRead;             // +0x000
        UINT16 InterceptCrWrite;            // +0x002
        UINT16 InterceptDrRead;             // +0x004
        UINT16 InterceptDrWrite;            // +0x006
        UINT32 InterceptException;          // +0x008
        UINT32 InterceptMisc1;              // +0x00c
        UINT32 InterceptMisc2;              // +0x010
        UINT8 Reserved1[0x03c - 0x014];     // +0x014
        UINT16 PauseFilterThreshold;        // +0x03c
        UINT16 PauseFilterCount;            // +0x03e
        UINT64 IopmBasePa;                  // +0x040
        UINT64 MsrpmBasePa;                 // +0x048
        UINT64 TscOffset;                   // +0x050
        UINT32 GuestAsid;                   // +0x058
        UINT32 TlbControl;                  // +0x05c
        UINT64 VIntr;                       // +0x060
        UINT64 InterruptShadow;             // +0x068
        UINT64 ExitCode;                    // +0x070
        UINT64 ExitInfo1;                   // +0x078
        UINT64 ExitInfo2;                   // +0x080
        UINT64 ExitIntInfo;                 // +0x088
        UINT64 NpEnable;                    // +0x090
        UINT64 AvicApicBar;                 // +0x098
        UINT64 GuestPaOfGhcb;               // +0x0a0
        UINT64 EventInj;                    // +0x0a8
        UINT64 NCr3;                        // +0x0b0
        UINT64 LbrVirtualizationEnable;     // +0x0b8
        UINT64 VmcbClean;                   // +0x0c0
        UINT64 NRip;                        // +0x0c8
        UINT8 NumOfBytesFetched;            // +0x0d0
        UINT8 GuestInstructionBytes[15];    // +0x0d1
        UINT64 AvicApicBackingPagePointer;  // +0x0e0
        UINT64 Reserved2;                   // +0x0e8
        UINT64 AvicLogicalTablePointer;     // +0x0f0
        UINT64 AvicPhysicalTablePointer;    // +0x0f8
        UINT64 Reserved3;                   // +0x100
        UINT64 VmcbSaveStatePointer;        // +0x108
        UINT8 Reserved4[0x400 - 0x110];     // +0x110
    } VMCB_CONTROL_AREA, * PVMCB_CONTROL_AREA;
    static_assert(sizeof(VMCB_CONTROL_AREA) == 0x400, "VMCB_CONTROL_AREA size mismatch");

    typedef struct _VMCB_STATE_SAVE_AREA
    {
        UINT16 EsSelector;                  // +0x000
        UINT16 EsAttrib;                    // +0x002
        UINT32 EsLimit;                     // +0x004
        UINT64 EsBase;                      // +0x008
        UINT16 CsSelector;                  // +0x010
        UINT16 CsAttrib;                    // +0x012
        UINT32 CsLimit;                     // +0x014
        UINT64 CsBase;                      // +0x018
        UINT16 SsSelector;                  // +0x020
        UINT16 SsAttrib;                    // +0x022
        UINT32 SsLimit;                     // +0x024
        UINT64 SsBase;                      // +0x028
        UINT16 DsSelector;                  // +0x030
        UINT16 DsAttrib;                    // +0x032
        UINT32 DsLimit;                     // +0x034
        UINT64 DsBase;                      // +0x038
        UINT16 FsSelector;                  // +0x040
        UINT16 FsAttrib;                    // +0x042
        UINT32 FsLimit;                     // +0x044
        UINT64 FsBase;                      // +0x048
        UINT16 GsSelector;                  // +0x050
        UINT16 GsAttrib;                    // +0x052
        UINT32 GsLimit;                     // +0x054
        UINT64 GsBase;                      // +0x058
        UINT16 GdtrSelector;                // +0x060
        UINT16 GdtrAttrib;                  // +0x062
        UINT32 GdtrLimit;                   // +0x064
        UINT64 GdtrBase;                    // +0x068
        UINT16 LdtrSelector;                // +0x070
        UINT16 LdtrAttrib;                  // +0x072
        UINT32 LdtrLimit;                   // +0x074
        UINT64 LdtrBase;                    // +0x078
        UINT16 IdtrSelector;                // +0x080
        UINT16 IdtrAttrib;                  // +0x082
        UINT32 IdtrLimit;                   // +0x084
        UINT64 IdtrBase;                    // +0x088
        UINT16 TrSelector;                  // +0x090
        UINT16 TrAttrib;                    // +0x092
        UINT32 TrLimit;                     // +0x094
        UINT64 TrBase;                      // +0x098
        UINT8 Reserved1[0x0cb - 0x0a0];     // +0x0a0
        UINT8 Cpl;                          // +0x0cb
        UINT32 Reserved2;                   // +0x0cc
        UINT64 Efer;                        // +0x0d0
        UINT8 Reserved3[0x148 - 0x0d8];     // +0x0d8
        UINT64 Cr4;                         // +0x148
        UINT64 Cr3;                         // +0x150
        UINT64 Cr0;                         // +0x158
        UINT64 Dr7;                         // +0x160
        UINT64 Dr6;                         // +0x168
        UINT64 Rflags;                      // +0x170
        UINT64 Rip;                         // +0x178
        UINT8 Reserved4[0x1d8 - 0x180];     // +0x180
        UINT64 Rsp;                         // +0x1d8
        UINT8 Reserved5[0x1f8 - 0x1e0];     // +0x1e0
        UINT64 Rax;                         // +0x1f8
        UINT64 Star;                        // +0x200
        UINT64 LStar;                       // +0x208
        UINT64 CStar;                       // +0x210
        UINT64 SfMask;                      // +0x218
        UINT64 KernelGsBase;                // +0x220
        UINT64 SysenterCs;                  // +0x228
        UINT64 SysenterEsp;                 // +0x230
        UINT64 SysenterEip;                 // +0x238
        UINT64 Cr2;                         // +0x240
        UINT8 Reserved6[0x268 - 0x248];     // +0x248
        UINT64 GPat;                        // +0x268
        UINT64 DbgCtl;                      // +0x270
        UINT64 BrFrom;                      // +0x278
        UINT64 BrTo;                        // +0x280
        UINT64 LastExcepFrom;               // +0x288
        UINT64 LastExcepTo;                 // +0x290
    } VMCB_STATE_SAVE_AREA, * PVMCB_STATE_SAVE_AREA;
    static_assert(sizeof(VMCB_STATE_SAVE_AREA) == 0x298, "VMCB_STATE_SAVE_AREA size mismatch");

    typedef struct _VMCB
    {
        VMCB_CONTROL_AREA ControlArea;
        VMCB_STATE_SAVE_AREA StateSaveArea;
        UINT8 Reserved1[0x1000 - sizeof(VMCB_CONTROL_AREA) - sizeof(VMCB_STATE_SAVE_AREA)];
    } VMCB, * PVMCB;
    static_assert(sizeof(VMCB) == 0x1000, "VMCB size mismatch");
}