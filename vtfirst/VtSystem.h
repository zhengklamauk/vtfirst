#ifndef __VTSYSTEM_H__
#define __VTSYSTEM_H__

#include <ntddk.h>

typedef struct _VMXREGION {
	PVOID				pvRegion;
	PHYSICAL_ADDRESS	pvPhysicalAddress;
}VMXREGION, *PVMXREGION;

VMXREGION g_VmxRegion;

NTSTATUS StartVirtualTechnology();
NTSTATUS StopVirtualTechnology();

static BOOLEAN IsSupportVMX();

#endif // !__VTSYSTEM_H__
