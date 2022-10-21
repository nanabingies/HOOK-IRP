
#include "device.hpp"
#include "driver.hpp"
#include "hook.hpp"


extern "C" NTSTATUS DriverEntry(_In_ DRIVER_OBJECT * DriverObject, _In_ UNICODE_STRING*) {

	PDEVICE_OBJECT device_object = nullptr;
	auto status = IoCreateDevice(DriverObject, 0, nullptr, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);
	if (!NT_SUCCESS(status))
		return STATUS_FAILED_DRIVER_ENTRY;

	DriverObject->DriverUnload = device::DriverUnload;

	UNICODE_STRING usString{};
	RtlInitUnicodeString(&usString, L"\\Device\\GIO");
	DRIVER_OBJECT* drv = nullptr;
	driver::GetDriverObject(usString, &drv);
	hook::PerformHook(drv);

	return STATUS_SUCCESS;
}

