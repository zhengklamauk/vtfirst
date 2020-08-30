#include <ntddk.h>
#include "VtSystem.h"

VOID DriverUnload(PDRIVER_OBJECT pDriverObj_)
{
    _StopVirtualTechnology();

	KdPrint(("Driver unload\n"));
	return;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj_, PUNICODE_STRING pus_Reg)
{
	KdPrint(("Driver entry\n"));

    _StartVirtualTechnology();

	pDriverObj_->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}