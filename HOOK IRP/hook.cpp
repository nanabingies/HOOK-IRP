#include "hook.hpp"

#pragma warning(disable : 4838)
#pragma warning(disable : 4244)
#pragma warning(disable : 4309)

namespace hook {

	DRIVER_OBJECT* driver_object;
	DRIVER_DISPATCH* IrpRead;
	DRIVER_DISPATCH* IrpWrite;

	PMDL mdl;
	PVOID BaseAddress;

	ULONGLONG guard;

	auto PerformHook(_In_ DRIVER_OBJECT* DriverObject) -> NTSTATUS {

		DbgPrint(__FUNCTION__" => \n");

		driver_object = DriverObject;

		IrpRead = (DRIVER_DISPATCH*)(DriverObject->MajorFunction[IRP_MJ_READ]);
		IrpWrite = (DRIVER_DISPATCH*)(DriverObject->MajorFunction[IRP_MJ_WRITE]);

		DbgPrint("IrpRead: 0x%p\n", (PVOID)IrpRead);
		DbgPrint("IrpWrite: 0x%p\n", (PVOID)IrpWrite);

		//map_page(IrpRead, sizeof PVOID);
		//PerformPatch(IrpRead, sizeof PVOID);

		ULONGLONG guard_icall = SetCfgDispatch(DriverObject, ULONGLONG(_ignore_icall));
		guard = guard_icall;

		DriverObject->MajorFunction[IRP_MJ_READ] = &HookRead;
		DriverObject->MajorFunction[IRP_MJ_WRITE] = &HookWrite;

		DbgPrint("Done Patching!!!!\n");

		return STATUS_SUCCESS;
	}

	auto RestoreHooks(_In_ DRIVER_OBJECT*) -> NTSTATUS {

		// re-enable cf guard
		SetCfgDispatch(driver_object, guard);
		driver_object->MajorFunction[IRP_MJ_READ] = IrpRead;
		driver_object->MajorFunction[IRP_MJ_WRITE] = IrpWrite;

		return STATUS_SUCCESS;
	}

	NTSTATUS map_page(_In_ PVOID VirtualAddress, _In_ ULONG size) {

		mdl = IoAllocateMdl(VirtualAddress, size, FALSE, FALSE, __nullptr);
		if (!mdl) {
			DbgPrint("[-] IoAllocateMdl failed.\n");
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		MmBuildMdlForNonPagedPool(mdl);

		// lock mdl pages in memory
		MmProbeAndLockPages(mdl, KernelMode, IoModifyAccess);

		// change the protection type for the memory address range.
		MmProtectMdlSystemAddress(mdl, PAGE_EXECUTE_READWRITE);

		BaseAddress = MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
		if (!BaseAddress) {
			DbgPrint("[-] MmMapLockedPagesSpecifyCache failed.\n");
			MmUnlockPages(mdl);
			IoFreeMdl(mdl);
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		DbgPrint("[+] Mapped mdl (0x%llx) to system virtual address : 0x%p\n", reinterpret_cast<ULONG64>(mdl), BaseAddress);

		MmUnmapLockedPages(BaseAddress, mdl);
		MmUnlockPages(mdl);
		IoFreeMdl(mdl);

		return STATUS_SUCCESS;
	}

	NTSTATUS PerformPatch(_In_ PVOID RoutineAddress, _In_ SIZE_T size) {

		//
		// jmp shellcode
		// the trampoline
		//
		CHAR shell_code[] = {
			// push rcx
			0x51,

			// movabs rcx, 0xOurFuncAddress (push into register as 64-bit value)
			0x48, 0xB9,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

			// xchg QWORD PTR [rsp], rcx (transpose operand)
			0x48, 0x87, 0x0C, 0x24,

			// ret
			0xC3
		};

		PVOID dstAddress = &HookRead;

		RtlCopyMemory(&shell_code[0x3], &dstAddress, size);

		// copy address of baseaddress to address of routine
		RtlCopyMemory(RoutineAddress, shell_code, sizeof shell_code);
		DbgPrint("RoutineAddress: %p\n", RoutineAddress);

		return STATUS_SUCCESS;
	}

	NTSTATUS HookRead(_In_ DEVICE_OBJECT* DeviceObject, _In_ IRP* Irp) {
		DbgPrint(__FUNCTION__" => Inside hooked function\n");
		DbgPrint(__FUNCTION__" <= \n");

		return IrpRead(DeviceObject, Irp);
	}

	NTSTATUS HookWrite(_In_ DEVICE_OBJECT* DeviceObject, _In_ IRP* Irp) {
		DbgPrint(__FUNCTION__" => Inside hooked function\n");
		DbgPrint(__FUNCTION__" <= \n");

		return IrpWrite(DeviceObject, Irp);
	}

	// https://github.com/not-wlan/driver-hijack/blob/master/memedriver/hijack.cpp
	ULONGLONG SetCfgDispatch(const PDRIVER_OBJECT driver, const ULONGLONG new_dispatch) {
		ULONG size = 0;
		const auto directory = PIMAGE_LOAD_CONFIG_DIRECTORY(RtlImageDirectoryEntryToData(driver->DriverStart, TRUE,
			IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, &size));

		if (directory != nullptr) {

			DbgPrint("IMAGE_LOAD_CONFIG_DIRECTORY found!\n");

			if (directory->GuardFlags & IMAGE_GUARD_CF_INSTRUMENTED) {
				{
					DbgPrint("CF Guard enabled! Patching.\n");
					const auto old_dispatch = directory->GuardCFDispatchFunctionPointer;

					auto cr0 = __readcr0();

					const auto old_cr0 = cr0;
					// disable write protection
					cr0 &= ~(1UL << 16);
					__writecr0(cr0);

					directory->GuardCFDispatchFunctionPointer = new_dispatch;

					__writecr0(old_cr0);

					DbgPrint("If you can read this CF Guard has been disabled.\n");

					return old_dispatch;
				}
			}
		}

		return 0;
	}
}