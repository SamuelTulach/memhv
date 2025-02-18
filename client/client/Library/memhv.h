#pragma once

/*
 * memhv
 * https://github.com/SamuelTulach | https://tulach.cc
 */

#include <intrin.h>
#include <Windows.h>
#include <iostream>
#include <stdint.h>

namespace HV
{
    namespace Shared
    {
        constexpr ULONG64 MAGIC = 0xfeed3;
        constexpr ULONG64 COMM_CHECK = 0xdead;

        constexpr ULONG64 MAX_RW_SIZE = 0x10000;

        enum CommandId
        {
            Invalid,
            CheckPresence,
            GetProcess,
            GetDirectoryBase,
            CopyProcessMemory,
            ProtectSelf,
        };

        enum ErrorCodes
        {
            Success,
            ControlBlockReadFail,
            MemoryCopyTooLarge,
            MemoryCopyFailSource,
            MemoryCopyFailTarget,
        };

        typedef struct _COPY_MEMORY_DATA
        {
            ULONG64 SourceDirectoryBase;
            ULONG64 SourceAddress;
            ULONG64 DestinationDirectoryBase;
            ULONG64 DestinationAddress;
            SIZE_T NumberOfBytes;
        } COPY_MEMORY_DATA;
    }

    namespace Data
    {
        inline bool Attached = false; // dont be too attached to females, they bite
        inline PVOID Shellcode = nullptr;

        inline ULONG64 CurrentEPROCESS = 0;
        inline ULONG64 CurrentDirectoryBase = 0;

        inline HANDLE TargetProcessHandle = nullptr;
        inline ULONG64 TargetEPROCESS = 0;
        inline ULONG64 TargetDirectoryBase = 0;
    }

    inline ULONG64 CallVM(ULONG64 arg1 = 0, ULONG64 arg2 = 0, ULONG64 arg3 = 0, ULONG64 arg4 = 0)
    {
        if (!Data::Shellcode)
        {
            // vmmcall
            // ret
            const BYTE code[] = { 0x0F, 0x01, 0xD9, 0xC3 };

            Data::Shellcode = VirtualAlloc(nullptr, sizeof(code), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (!Data::Shellcode)
                throw std::exception("Out of memory");

            memcpy(Data::Shellcode, code, sizeof(code));
        }

        typedef ULONG64(__stdcall* CallFunc)(ULONG64 arg1, ULONG64 arg2, ULONG64 arg3, ULONG64 arg4);
        const CallFunc func = static_cast<CallFunc>(Data::Shellcode);

        ULONG64 output;
        __try
        {
            output = func(arg1, arg2, arg3, arg4);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            printf("[!] SEH handler called\n");
            output = 0xAABB;
        }

        return output;
    }

    inline bool Error(ULONG64 hvOutput)
    {
        if (hvOutput == 0xAABB)
            return true; // HV not loaded

        if (hvOutput == 0xFFFF)
            return true; // command not implemented

        if (!hvOutput)
            return true; // empty output

        return false;
    }

    inline bool CheckPresence()
    {
        const ULONG64 output = CallVM(Shared::MAGIC, Shared::CommandId::CheckPresence);
        return output == Shared::COMM_CHECK;
    }

    inline bool Protect()
    {
        const ULONG64 output = CallVM(Shared::MAGIC, Shared::CommandId::ProtectSelf);
        return output == Shared::ErrorCodes::Success;
    }

    inline ULONG64 GetEPROCESS(UINT32 processId)
    {
        return CallVM(Shared::MAGIC, Shared::CommandId::GetProcess, processId);
    }

    inline ULONG64 GetDirbase(ULONG64 targetProcess)
    {
        return CallVM(Shared::MAGIC, Shared::CommandId::GetDirectoryBase, targetProcess);
    }

    inline bool AttachToProcess(UINT32 processId)
    {
        Data::CurrentEPROCESS = GetEPROCESS(GetCurrentProcessId());
        if (Error(Data::CurrentEPROCESS))
            return false;

        Data::TargetEPROCESS = GetEPROCESS(processId);
        if (Error(Data::TargetEPROCESS))
            return false;

        Data::CurrentDirectoryBase = GetDirbase(Data::CurrentEPROCESS);
        if (Error(Data::CurrentDirectoryBase))
            return false;

        Data::TargetDirectoryBase = GetDirbase(Data::TargetEPROCESS);
        if (Error(Data::TargetDirectoryBase))
            return false;

        Data::TargetProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, processId);
        if (!Data::TargetProcessHandle || Data::TargetProcessHandle == INVALID_HANDLE_VALUE)
            return false;

        Data::Attached = true;
        return true;
    }

    inline bool CopyProcessMemory(ULONG64 sourceDirbase, ULONG64 sourceAddress, ULONG64 targetDirbase, ULONG64 targetAddress, SIZE_T bytes)
    {
        Shared::COPY_MEMORY_DATA data = { 0 };
        data.SourceDirectoryBase = sourceDirbase;
        data.SourceAddress = sourceAddress;
        data.DestinationDirectoryBase = targetDirbase;
        data.DestinationAddress = targetAddress;
        data.NumberOfBytes = bytes;

        const ULONG64 value = CallVM(Shared::MAGIC, Shared::CommandId::CopyProcessMemory, reinterpret_cast<ULONG64>(&data), Data::CurrentDirectoryBase);
        return value == Shared::ErrorCodes::Success;
    }

    inline bool ReadMemory(ULONG64 source, ULONG64 buffer, SIZE_T size)
    {
        memset(reinterpret_cast<PVOID>(buffer), 0, size);
        return CopyProcessMemory(Data::TargetDirectoryBase, source, Data::CurrentDirectoryBase, buffer, size);
    }

    inline bool WriteMemory(ULONG64 buffer, ULONG64 destination, SIZE_T size)
    {
        return CopyProcessMemory(Data::CurrentDirectoryBase, buffer, Data::TargetDirectoryBase, destination, size);
    }

    template<typename T>
    T Read(ULONG64 address)
    {
        T value = T();
        ReadMemory(address, (ULONG64)&value, sizeof(T));
        return value;
    }

    template<typename T>
    void Write(ULONG64 address, T value)
    {
        WriteMemory((ULONG64)&value, address, sizeof(T));
    }
}