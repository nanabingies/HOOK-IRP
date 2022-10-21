#pragma once

#include "Header.hpp"

namespace driver {
	auto GetDriverName(_In_ DRIVER_OBJECT*, _In_ UNICODE_STRING, _Out_ UNICODE_STRING*) -> NTSTATUS;

	auto GetDriverObject(_In_ UNICODE_STRING, _Out_ PDRIVER_OBJECT*) -> NTSTATUS;
}
