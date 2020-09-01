#include "GuestEntry.h"
#include "Vmx.h"

void __declspec(naked) _GuestEntry()
{
    __asm {
        mov ax, ss;
        mov ss, ax;
        mov ax, ds;
        mov ds, ax;
        mov ax, es;
        mov es, ax;
        mov ax, fs;
        mov fs, ax;
        mov ax, gs;
        mov gs, ax;
    }

    _vmcall();
}