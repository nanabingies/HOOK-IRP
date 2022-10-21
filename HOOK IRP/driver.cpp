#include "driver.hpp"

namespace driver {

	auto GetDriverName(_In_ DRIVER_OBJECT* driver_object, _In_ UNICODE_STRING driver_string, _Out_ UNICODE_STRING* driver_name) -> NTSTATUS {

		DbgPrint(__FUNCTION__" => \n");

		*driver_name = { 0 };
		BOOLEAN found = FALSE;

		auto DriverSection = *(PVOID*)((PUCHAR)driver_object->DriverSection);
		if (DriverSection == nullptr)
			return STATUS_UNSUCCESSFUL;

		auto LdrDataTableEntry = (LDR_DATA_TABLE_ENTRY*)DriverSection;

		for (auto* entry = LdrDataTableEntry->InLoadOrderLinks.Flink;
			entry != &LdrDataTableEntry->InLoadOrderLinks; entry = entry->Flink) {
			auto Ldr = (LDR_DATA_TABLE_ENTRY*)(CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks));
			
			if (FsRtlIsNameInExpression(&driver_string, &Ldr->BaseDllName, TRUE, nullptr) == TRUE) {
				driver_name = &Ldr->BaseDllName;
				DbgPrint("Found it %wZ\n", driver_name);
				found = TRUE;
				break;
			}
		}

		DbgPrint(__FUNCTION__" <= \n");

		return found == TRUE ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
	}

	auto GetDriverObject(_In_ UNICODE_STRING DriverName, _Out_ PDRIVER_OBJECT* DriverObject) -> NTSTATUS {

		DbgPrint(__FUNCTION__" => \n");

		*DriverObject = nullptr;
		PDEVICE_OBJECT DevObj = nullptr;
		PFILE_OBJECT FileObj = nullptr;

		if (DriverName.Buffer == nullptr || DriverName.Length <= 0) {
			DbgPrint("[-] DriverName parameter cannot be empty\n");
			return STATUS_INVALID_PARAMETER;
		}

		auto status = IoGetDeviceObjectPointer(&DriverName, FILE_ALL_ACCESS, &FileObj, &DevObj);
		if (!NT_SUCCESS(status)) {
			DbgPrint("[-] IoGetDeviceObjectPointer failed with error code: %x\n", status);
			return status;
		}

		*DriverObject = DevObj->DriverObject;

		DbgPrint(__FUNCTION__" <= \n");

		return STATUS_SUCCESS;
	}
}
