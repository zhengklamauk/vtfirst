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

    g_GuestRegs.esp_ = _vmread(GUEST_RSP);
    g_GuestRegs.eip_ = _vmread(GUEST_RSP);

    KdPrint(("g_Guest.Regs.esp_=%X\ng_Guest.Regs.eip_=%X\n", g_GuestRegs.esp_, g_GuestRegs.eip_));

    __asm int 3

    return;
}