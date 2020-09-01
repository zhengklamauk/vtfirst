#include <ntddk.h>
#include "VtSystem.h"
#include "HostHandler.h"
#include "GuestEntry.h"

VOID DriverUnload(PDRIVER_OBJECT pDriverObj_)
{
    _StopVirtualTechnology();

	KdPrint(("Driver unload\n"));
	return;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj_, PUNICODE_STRING pus_Reg)
{
    pDriverObj_->DriverUnload = DriverUnload;
	KdPrint(("Driver entry\n"));

    NTSTATUS status = _StartVirtualTechnology();
    if (status == STATUS_SUCCESS) {
        _LaunchGuest(_HostHander, _GuestEntry);
    }

	return STATUS_SUCCESS;
}