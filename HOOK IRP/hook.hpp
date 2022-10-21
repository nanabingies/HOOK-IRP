#pragma once

#include "Header.hpp"


namespace hook {

	auto PerformHook(_In_ DRIVER_OBJECT*) -> NTSTATUS;
	
	auto RestoreHooks(_In_ DRIVER_OBJECT*) -> NTSTATUS;

	NTSTATUS HookRead(_In_ DEVICE_OBJECT*, _In_ IRP*);
	NTSTATUS HookWrite(_In_ DEVICE_OBJECT*, _In_ IRP*);

	NTSTATUS map_page(_In_ PVOID, _In_ ULONG);
	NTSTATUS PerformPatch(_In_ PVOID, _In_ SIZE_T);

	ULONGLONG SetCfgDispatch(const PDRIVER_OBJECT, const ULONGLONG);
}