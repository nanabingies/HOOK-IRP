#include "device.hpp"

namespace device {

	extern "C" VOID DriverUnload(_In_ DRIVER_OBJECT * DriverObject) {
		DbgPrint(__FUNCTION__" => \n");
		IoDeleteDevice(DriverObject->DeviceObject);

		//driver_object = nullptr;
	}
}