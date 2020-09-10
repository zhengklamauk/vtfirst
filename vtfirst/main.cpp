#include <ntddk.h>
#include "VtSystem.h"
#include "HostHandler.h"
#include "GuestEntry.h"

VMXRETURN g_VMXReturn;

DWORD32 g_Test;

VOID DriverUnload(PDRIVER_OBJECT pDriverObj_)
{
    /*__asm {
        pushad;
        pushfd;

        mov g_VMXReturn.HostReturnEip, offset __HostRet;
        mov g_VMXReturn.HostReturnEsp, esp;
    }*/
    _StopVirtualTechnology();
    /*__asm {
    __HostRet:
        popfd;
        popad;
    }*/

    g_Test = 0x20;
	KdPrint(("Driver unload\n"));
	return;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj_, PUNICODE_STRING pus_Reg)
{
    pDriverObj_->DriverUnload = DriverUnload;
	KdPrint(("Driver entry\nTest: %X\n", &g_Test));

    DWORD64 dw64Address = (DWORD64)_SetupEPT();
    if (dw64Address == 0) {
        KdPrint(("EPT enable error\n"));
    }
    else {
        KdPrint(("EPT ON\n"));
    }

    //__asm int 3;
    NTSTATUS status = _StartVirtualTechnology();
    if (status != STATUS_SUCCESS) {
        goto __RET;
    }

    __asm {
        pushad;
        pushfd;

        mov g_VMXReturn.GuestReturnEip, offset __GuestRet;
        mov g_VMXReturn.GuestReturnEsp, esp;
    }
    _LaunchGuest(_HostHander, _GuestEntry);
    __asm {
    __GuestRet:
        popfd;
        popad;
    }

    g_Test = 0x10;

__RET:
	return STATUS_SUCCESS;
}