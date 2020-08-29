#include <ntddk.h>

VOID DriverUnload(PDRIVER_OBJECT pDriverObj_)
{
	KdPrint(("Driver unload\n"));
	return;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj_, PUNICODE_STRING pus_Reg)
{
	pDriverObj_->DriverUnload = DriverUnload;
	KdPrint(("Driver entry\n"));
	return STATUS_SUCCESS;
}