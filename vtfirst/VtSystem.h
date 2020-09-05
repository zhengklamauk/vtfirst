#ifndef __VTSYSTEM_H__
#define __VTSYSTEM_H__

#include <ntddk.h>
#include "Vmx.h"

#define HOST_STACK_SIZE             0x2000
#define GUEST_STACK_SIZE            0x2000

typedef struct _VMXINFORMATION {
	PVOID				pvHostAddress;
	PHYSICAL_ADDRESS	pvHostPhysicalAddress;
	BOOLEAN				isVMXON;

    PVOID               pvHostStack;
    PVOID               pvGuestStack;
    PVOID               pvVMCSAddress;
    PHYSICAL_ADDRESS    pvVMCSPhysicalAddress;
}VMXINFORMATION, *PVMXINFORMATION;

NTSTATUS _StartVirtualTechnology();
BOOLEAN _LaunchGuest(PVOID pvHostHandler_, PVOID pvGuestEntry_);
VOID _StopVirtualTechnology();

static BOOLEAN _IsSupportVMX();
static BOOLEAN _CreateHostRegion(PVMXINFORMATION pstVMXInformation_);
static BOOLEAN _CreateVMCSRegion(PVMXINFORMATION pstVMXInformation_);
static void _SetupVMCS(PVOID pvHostHandler_, PVOID pvGuestEntry_, PVMXINFORMATION pstVMXInformation_);
static DWORD32 _AdjustControlValue(DWORD32 dw32Original_, DWORD32 dw32MsrIndex_);
static void _FreeVMXInformationMember(PVMXINFORMATION pstVMXInformation_);

#endif // !__VTSYSTEM_H__
