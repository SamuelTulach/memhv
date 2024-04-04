#include "../Global.h"

constexpr UINT32 PageCount = 64;
static Memory::PAGE_INFO PageList[PageCount];

ULONG64 Memory::GetDirectoryBase(const PEPROCESS process)
{
    return *reinterpret_cast<ULONG64*>(reinterpret_cast<PBYTE>(process) + 0x28);
}

ULONG64 Memory::VirtualToPhysical(const ULONG64 virtualAddress)
{
    return MmGetPhysicalAddress(reinterpret_cast<PVOID>(virtualAddress)).QuadPart;
}

ULONG64 Memory::PhysicalToVirtual(const ULONG64 physicalAddress)
{
    PHYSICAL_ADDRESS physical;
    physical.QuadPart = physicalAddress;
    return reinterpret_cast<ULONG64>(MmGetVirtualForPhysical(physical));
}

ULONG64 Memory::ResolveProcessPhysicalAddress(const UINT32 pageIndex, const ULONG64 directoryBase, const ULONG64 virtualAddress)
{
    UNREFERENCED_PARAMETER(pageIndex);

    const ULONG64 original = __readcr3();
    __writecr3(directoryBase);

    ULONG64 result = VirtualToPhysical(virtualAddress);

    __writecr3(original);

    return result;
}

Memory::PTE* Memory::GetPte(const ULONG64 address)
{
    VIRTUAL_ADDRESS virtualAddress;
    virtualAddress.Value = address;

    PTE_CR3 cr3;
    cr3.Value = __readcr3();

    PML4E* pml4 = reinterpret_cast<PML4E*>(PhysicalToVirtual(PFN_TO_PAGE(cr3.Pml4)));
    const PML4E* pml4e = (pml4 + virtualAddress.Pml4Index);
    if (!pml4e->Present)
        return nullptr;

    PDPTE* pdpt = reinterpret_cast<PDPTE*>(PhysicalToVirtual(PFN_TO_PAGE(pml4e->Pdpt)));
    const PDPTE* pdpte = pdpte = (pdpt + virtualAddress.PdptIndex);
    if (!pdpte->Present)
        return nullptr;

    // sanity check 1GB page
    if (pdpte->PageSize)
        return nullptr;

    PDE* pd = reinterpret_cast<PDE*>(PhysicalToVirtual(PFN_TO_PAGE(pdpte->Pd)));
    const PDE* pde = pde = (pd + virtualAddress.PdIndex);
    if (!pde->Present)
        return nullptr;

    // sanity check 2MB page
    if (pde->PageSize)
        return nullptr;

    PTE* pt = reinterpret_cast<PTE*>(PhysicalToVirtual(PFN_TO_PAGE(pde->Pt)));
    PTE* pte = (pt + virtualAddress.PtIndex);
    if (!pte->Present)
        return nullptr;

    return pte;
}

bool Memory::PreparePage(PAGE_INFO* targetPage)
{
    PHYSICAL_ADDRESS maxAddress;
    maxAddress.QuadPart = MAXULONG64;

    targetPage->VirtualAddress = MmAllocateContiguousMemory(PAGE_SIZE, maxAddress);
    if (!targetPage->VirtualAddress)
        return false;

    targetPage->CopyBuffer = ExAllocatePool(NonPagedPool, Shared::MAX_RW_SIZE);
    if (!targetPage->CopyBuffer)
        return false;

    targetPage->PageEntry = GetPte(reinterpret_cast<ULONG64>(targetPage->VirtualAddress));
    if (!targetPage->PageEntry)
        return false;

    return true;
}

bool Memory::PreparePages()
{
    for (UINT32 i = 0; i < PageCount; i++)
    {
        if (!PreparePage(&PageList[i]))
            return false;
    }

    return true;
}

PVOID Memory::OverwritePage(const UINT32 pageIndex, const ULONG64 physicalAddress)
{
    const ULONG pageOffset = physicalAddress % PAGE_SIZE;
    const ULONG64 pageStartPhysical = physicalAddress - pageOffset;

    PAGE_INFO* pageInfo = &PageList[pageIndex];
    pageInfo->PreviousPageFrame = pageInfo->PageEntry->PageFrame;
    pageInfo->PageEntry->PageFrame = PAGE_TO_PFN(pageStartPhysical);
    __invlpg(pageInfo->VirtualAddress);

    return reinterpret_cast<PVOID>(reinterpret_cast<ULONG64>(pageInfo->VirtualAddress) + pageOffset);
}

void Memory::RestorePage(const UINT32 pageIndex)
{
    const PAGE_INFO* pageInfo = &PageList[pageIndex];
    pageInfo->PageEntry->PageFrame = pageInfo->PreviousPageFrame;
    __invlpg(pageInfo->VirtualAddress);
}

void Memory::ReadPhysicalAddress(const UINT32 pageIndex, const ULONG64 targetAddress, const PVOID buffer, const SIZE_T size)
{
    const PVOID virtualAddress = OverwritePage(pageIndex, targetAddress);
    memcpy(buffer, virtualAddress, size);
    RestorePage(pageIndex);
}

void Memory::WritePhysicalAddress(const UINT32 pageIndex, const ULONG64 targetAddress, const PVOID buffer, const SIZE_T size)
{
    const PVOID virtualAddress = OverwritePage(pageIndex, targetAddress);
    memcpy(virtualAddress, buffer, size);
    RestorePage(pageIndex);
}

NTSTATUS Memory::ReadProcessMemory(const UINT32 pageIndex, const ULONG64 directoryBase, const ULONG64 address, PVOID buffer, const SIZE_T size, SIZE_T* bytesRead)
{
    SIZE_T currentOffset = 0;
    SIZE_T totalSize = size;
    while (totalSize)
    {
        const ULONG64 currentPhysicalAddress = ResolveProcessPhysicalAddress(pageIndex, directoryBase, address + currentOffset);
        if (!currentPhysicalAddress)
            return STATUS_NOT_FOUND;

        const ULONG64 readSize = min(PAGE_SIZE - (currentPhysicalAddress & 0xFFF), totalSize);

        ReadPhysicalAddress(pageIndex, currentPhysicalAddress, reinterpret_cast<PVOID>(reinterpret_cast<ULONG64>(buffer) + currentOffset), readSize);

        totalSize -= readSize;
        currentOffset += readSize;
    }

    *bytesRead = currentOffset;

    return STATUS_SUCCESS;
}

NTSTATUS Memory::WriteProcessMemory(const UINT32 pageIndex, const ULONG64 directoryBase, const ULONG64 address, PVOID buffer, const SIZE_T size, SIZE_T* bytesWritten)
{
    SIZE_T currentOffset = 0;
    SIZE_T totalSize = size;
    while (totalSize)
    {
        const ULONG64 currentPhysicalAddress = ResolveProcessPhysicalAddress(pageIndex, directoryBase, address + currentOffset);
        if (!currentPhysicalAddress)
            return STATUS_NOT_FOUND;

        const ULONG64 writeSize = min(PAGE_SIZE - (currentPhysicalAddress & 0xFFF), totalSize);

        WritePhysicalAddress(pageIndex, currentPhysicalAddress, reinterpret_cast<PVOID>(reinterpret_cast<ULONG64>(buffer) + currentOffset), writeSize);

        totalSize -= writeSize;
        currentOffset += writeSize;
    }

    *bytesWritten = currentOffset;

    return STATUS_SUCCESS;
}

NTSTATUS Memory::CopyProcessMemory(const UINT32 pageIndex, const ULONG64 sourceDirectoryBase, const ULONG64 sourceAddress, const ULONG64 targetDirectoryBase, const ULONG64 targetAddress, const SIZE_T bufferSize)
{
    if (!Utils::ValidUsermodeAddress(sourceAddress))
        return STATUS_INVALID_ADDRESS;

    if (!Utils::ValidUsermodeAddress(targetAddress))
        return STATUS_INVALID_ADDRESS;

    const PAGE_INFO* pageInfo = &PageList[pageIndex];

    SIZE_T bytes = 0;
    NTSTATUS status = ReadProcessMemory(pageIndex, sourceDirectoryBase, sourceAddress, pageInfo->CopyBuffer, bufferSize, &bytes);
    if (!NT_SUCCESS(status))
        return STATUS_ABANDONED;

    return WriteProcessMemory(pageIndex, targetDirectoryBase, targetAddress, pageInfo->CopyBuffer, bufferSize, &bytes);
}