<p align="center">
    <img width="100px" height="auto" src="assets/chip-icon.png" />
    <h3 align="center">memhv</h3>
    <p align="center"><i>Minimalistic hypervisor with memory introspection capabilities</i></p>
</p>

## About
This project has a single goal: to be as minimal as possible while providing a hypercall API for reading/writing an address space of any (protected) process. It is a standalone Microsoft Windows kernel-mode driver that can be loaded either normally or through manual mapping.

![screenshot](assets/screenshot.png)

## Support
- Windows 10 or Windows 11 (both 64-bit, [offsets may need updating](https://github.com/SamuelTulach/memhv/blob/main/memhv/memhv/Source/Utils.cpp#L170))
- AMD processor with SVM and NPT support

## Usage
1. Ensure that you have SVM enabled in UEFI firmware options (BIOS)
2. Make sure Microsoft Hyper-V is fully disabled
3. Use manual mapper to load the kernel mode driver ([kdmapper](https://github.com/TheCruZ/kdmapper), [KDU](https://github.com/hfiref0x/KDU))
4. Enjoy hypercall API (see client folder)

## Detection vectors
Common timing attacks are ineffective against this hypervisor, as it does not exit on CPUID or similar instructions typically used in such attacks. When manually mapping, code will be in a memory region which is not associated with any legitimate module, having all the usual vectors. However, this can be easily mitigated by adjusting the NPT to completely hide the hypervisor memory from the guest (**this is not implemented in this project, you have to do it yourself**).

At the time of release, no popular anti-cheat has issues with this hypervisor running.

## FAQ
- **Q:** Will there be a version supporting Intel CPUs / Intel VT-x?
- **A:** No, at least not public.
- **Q:** Driver is returning unsuccessful status and the hypervisor is not loaded, why is that?
- **A:** If virtualization is truly enabled in BIOS, then make sure Hyper-V is really disabled. It might be off in Windows Features dialog, but still running due to WSL, Docker, VBS/HVCI and similar. 
- **Q:** Hypervisor loads just fine, but the moment I try to attach to a process, the system crashes!
- **A:** Certain system APIs cannot be used in the VM-exit context, so the EPROCESS of the target process is found manually by iterating a linked list. Hard-coded offset is used. Make sure it is updated.
- **Q:** Hypervisor loads fine, everything works, but after some time the system crashes!
- **A:** System sleep/hibernation is not supported. Memory mapping is done only for the first 512 GB of physical memory. In theory, that should be fine, but in reality, certain drivers (most commonly overclocking tools, motherboard utilities) will attempt to access memory beyond this range. Since there is no mapping, a nested page fault will occur, causing a system crash. You can fix this by implementing an exit handler that will add memory mappings on the fly, or if you don't care about 1.4 GB of useless space, then you can map all the 512 PML4 entries in NPT at once.
- **Q:** System crashes/reboots when I open a game with X anti-cheat!
- **A:** Make sure NESTED_MODE is set to false in Global.h, it should be true only if you are testing the HV inside of virtual machine like VMware.
- **Q:** Do you have anything less barebones then this?
- **A:** I have [Sphinx project](https://youtu.be/ocdVPpKP110) already mentioned above. It also hides its memory, supports Intel VT-x, reading of guarded pages, proper TSC offsetting, CR3 resolving (for processes with trashed directory base), etc. I have no plans to publish it at the moment.

## Credits
- [SimpleSvm](https://github.com/tandasat/SimpleSvm) by @tandasat
