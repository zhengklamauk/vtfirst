#include "HostHandler.h"
#include "Vmx.h"

GUESTREGS g_GuestRegs;

void __declspec(naked) _HostHander()
{
    _VMMEntryPointEbd();
}

static void _VMMEntryPointEbd()
{
    DWORD32 dw32ExitReason;

	dw32ExitReason = _vmread(VM_EXIT_REASON);

    g_GuestRegs.esp_ = _vmread(GUEST_RSP);
    g_GuestRegs.eip_ = _vmread(GUEST_RIP);

    KdPrint(("dw32ExitReason=%X\ng_Guest.Regs.esp_=%X\ng_Guest.Regs.eip_=%X\n", dw32ExitReason, g_GuestRegs.esp_, g_GuestRegs.eip_));

    __asm int 3

    return;
}