#pragma once

#include "Header.hpp"

namespace device {
	extern "C" NTSTATUS IrpCreate(_In_ DEVICE_OBJECT*, _In_ IRP*);
	extern "C" NTSTATUS IrpClose(_In_ DEVICE_OBJECT*, _In_ IRP*);
	extern "C" NTSTATUS IrpDevControl(_In_ DEVICE_OBJECT*, _In_ IRP*);

	extern "C" VOID DriverUnload(_In_ DRIVER_OBJECT*);

	//DEVICE_OBJECT* device_object = nullptr;
	//DRIVER_OBJECT* driver_object = nullptr;
}