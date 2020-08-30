#ifndef __VTSYSTEM_H__
#define __VTSYSTEM_H__

#include <ntddk.h>

typedef struct _VMXREGION {
	PVOID				pvRegion;
	PHYSICAL_ADDRESS	pvPhysicalAddress;
	BOOLEAN				isVMXON;
}VMXREGION, *PVMXREGION;

NTSTATUS _StartVirtualTechnology();
VOID _StopVirtualTechnology();

static BOOLEAN _IsSupportVMX();
static BOOLEAN _CreateVMXRegion(PVMXREGION pstVMXRegion_);

#endif // !__VTSYSTEM_H__
