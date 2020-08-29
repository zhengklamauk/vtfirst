#include "VtSystem.h"
#include "RegStruct.h"
#include "Register.h"
#include "utility.h"
#include "Vmx.h"


NTSTATUS StartVirtualTechnology()
{
	if (!IsSupportVMX()) {
		KdPrint(("No support VMX\n"));
		return STATUS_UNSUCCESSFUL;
	}

	_CR4 stCr4 = { 0 };
	*(DWORD32*)&stCr4 = GetCr4();
	stCr4.VMXE = 1;
	SetCr4(*(DWORD32*)&stCr4);

	g_VmxRegion.pvRegion = ExAllocatePoolWithTag(NonPagedPool, 0x1000, 'VT01');
	if (g_VmxRegion.pvRegion == NULL) {
		_KdPrint(("ExAllocatePoolWithTag\n"));
		return STATUS_UNSUCCESSFUL;
	}
	RtlZeroMemory(g_VmxRegion.pvRegion, 0x1000);
	g_VmxRegion.pvPhysicalAddress = MmGetPhysicalAddress(g_VmxRegion.pvRegion);
	_vmxon(g_VmxRegion.pvPhysicalAddress.LowPart, g_VmxRegion.pvPhysicalAddress.HighPart);

	_EFLAGS stEflags = { 0 };
	*(DWORD32*)&stEflags = GetEflags();
	if (stEflags.CF != 0) {
		_KdPrint(("vmxon error\n"));

		if (g_VmxRegion.pvRegion != NULL) {
			ExFreePool(g_VmxRegion.pvRegion);
			g_VmxRegion.pvRegion = NULL;
			g_VmxRegion.pvPhysicalAddress.QuadPart = NULL;
		}
	}

	KdPrint(("VMX ON\n"));
	return STATUS_SUCCESS;
}

NTSTATUS StopVirtualTechnology()
{
	_vmxoff();

	_CR4 stCr4 = { 0 };
	*(DWORD32*)&stCr4 = GetCr4();
	stCr4.VMXE = 0;
	SetCr4(*(DWORD32*)&stCr4);

	if (g_VmxRegion.pvRegion != NULL) {
		ExFreePool(g_VmxRegion.pvRegion);
		g_VmxRegion.pvRegion = NULL;
		g_VmxRegion.pvPhysicalAddress.QuadPart = NULL;
	}

	KdPrint(("VMX OFF\n"));
	return STATUS_SUCCESS;
}

static BOOLEAN IsSupportVMX()
{
	return TRUE;
}