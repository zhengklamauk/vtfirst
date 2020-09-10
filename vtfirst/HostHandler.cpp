#include "HostHandler.h"

GUESTREGS g_GuestRegs;

extern BOOLEAN g_isStopVMX;
extern VMXRETURN g_VMXReturn;
extern VMXSTOP g_VMXStop;
extern VMXINFORMATION g_VMXInformation;

void __declspec(naked) _HostHander()
{
    __asm {
        mov g_GuestRegs.eax_, eax;
        mov g_GuestRegs.ecx_, ecx;
        mov g_GuestRegs.edx_, edx;
        mov g_GuestRegs.ebx_, ebx;
        mov g_GuestRegs.esi_, esi;
        mov g_GuestRegs.edi_, edi;
        mov g_GuestRegs.ebp_, ebp;
    }
    _VMMEntryPointEbd();

    __asm {
        mov eax, g_GuestRegs.eax_;
        mov ecx, g_GuestRegs.ecx_;
        mov edx, g_GuestRegs.edx_;
        mov ebx, g_GuestRegs.ebx_;
        
        mov esi, g_GuestRegs.esi_;
        mov edi, g_GuestRegs.edi_;
        mov ebp, g_GuestRegs.ebp_;
    }
    _vmresume();
}

static void _VMMEntryPointEbd()
{
    DWORD32 dw32ExitReason;
    DWORD32 dw32ExitStructionLen;
    DWORD32 dw32GuestResumeEip;

	dw32ExitReason = _vmread(VM_EXIT_REASON);
    dw32ExitStructionLen = _vmread(VM_EXIT_INSTRUCTION_LEN);

    g_GuestRegs.eflags_ = _vmread(GUEST_RFLAGS);
    g_GuestRegs.esp_ = _vmread(GUEST_RSP);
    g_GuestRegs.eip_ = _vmread(GUEST_RIP);

    //KdPrint(("dw32ExitReason=%X\ng_Guest.Regs.esp_=%X\ng_Guest.Regs.eip_=%X\n", dw32ExitReason, g_GuestRegs.esp_, g_GuestRegs.eip_));

    //__asm int 3
    switch (dw32ExitReason) {
    case EXIT_REASON_CPUID:
        KdPrint(("call cpuid\n"));
        _HandleCpuid();
        break;

    case EXIT_REASON_CR_ACCESS:
        _HandleCrAccess();
        break;

    case EXIT_REASON_VMCALL:
        KdPrint(("call vmcall\n"));
        _HandleVmcall();
        break;

    case EXIT_EPT_VIOLATION:
        //__asm int 3;
        _HandleEPTViolation();
        dw32ExitStructionLen = 0;
        break;

    default:
        KdPrint(("Not handled reason: %X\ng_GuestRegs.eip_=%X\n", dw32ExitReason, g_GuestRegs.eip_));
        __asm int 3;        //其它未处理的退出原因
    }

    dw32GuestResumeEip = g_GuestRegs.eip_ + dw32ExitStructionLen;
    _vmwrite(GUEST_RIP, dw32GuestResumeEip);
    _vmwrite(GUEST_RSP, g_GuestRegs.esp_);
    _vmwrite(GUEST_RFLAGS, g_GuestRegs.eflags_);

    return;
}

static void _HandleCpuid()
{
    CPUIDRESULT stCpuidResult;

    //测试用
    if (g_GuestRegs.ecx_ == 'Mini') { //'Mini'=0x4D696E69
        g_GuestRegs.ecx_ = 0x11111111;
        g_GuestRegs.edx_ = 0x99999999;
        g_GuestRegs.ebx_ = 0x12345678;
        return;
    }

    _cpuid(g_GuestRegs.ecx_, &stCpuidResult);
    g_GuestRegs.eax_ = stCpuidResult.eax_;
    g_GuestRegs.ecx_ = stCpuidResult.ecx_;
    g_GuestRegs.edx_ = stCpuidResult.edx_;
    g_GuestRegs.ebx_ = stCpuidResult.ebx_;

    return;
}

static void _HandleCrAccess()
{
    EXITREASON_CRACCESS stExitReasonCrAccess;

    stExitReasonCrAccess.u.LowPart = _vmread(EXIT_QUALIFICATION);

    if (stExitReasonCrAccess.CRX != 3) {
        __asm int 3;    //不是CR3的情况先让其断下来
    }

    switch (stExitReasonCrAccess.AccessType) {
    case 0:
        _MoveToCr(&stExitReasonCrAccess);
        break;

    case 1:
        _MoveFromCr(&stExitReasonCrAccess);
        break;
    }

    return;
}

static void _HandleVmcall()
{
    if (g_isStopVMX == TRUE) {
        _vmclear(g_VMXInformation.pvVMCSPhysicalAddress.LowPart, g_VMXInformation.pvVMCSPhysicalAddress.HighPart);
        _vmxoff();

        __asm {
            mov esp, g_VMXStop.ReturnEsp;
            jmp g_VMXStop.ReturnEip;
        }
    }
    else {
        __asm int 3;    //其它情况暂让它断下
    }

    
    return;
}

static void _MoveToCr(PEXITREASON_CRACCESS pstExitReasonCrAccess)
{
    DWORD32 dw32Tmp = *((DWORD32*)&g_GuestRegs + pstExitReasonCrAccess->Reg);

    switch (pstExitReasonCrAccess->CRX) {
    case 3:
        _vmwrite(GUEST_CR3, dw32Tmp);
        //DWORD32 dw32Cr3 = _GetCr3();
        _SetCr3(_vmread(GUEST_CR3));

        _vmwrite(GUEST_PDPTR0, MmGetPhysicalAddress((PVOID)0xC0600000).LowPart | 1);
        _vmwrite(GUEST_PDPTR0_HIGH, MmGetPhysicalAddress((PVOID)0xC0600000).HighPart);
        _vmwrite(GUEST_PDPTR1, MmGetPhysicalAddress((PVOID)0xC0601000).LowPart | 1);
        _vmwrite(GUEST_PDPTR1_HIGH, MmGetPhysicalAddress((PVOID)0xC0601000).HighPart);
        _vmwrite(GUEST_PDPTR2, MmGetPhysicalAddress((PVOID)0xC0602000).LowPart | 1);
        _vmwrite(GUEST_PDPTR2_HIGH, MmGetPhysicalAddress((PVOID)0xC0602000).HighPart);
        _vmwrite(GUEST_PDPTR3, MmGetPhysicalAddress((PVOID)0xC0603000).LowPart | 1);
        _vmwrite(GUEST_PDPTR3_HIGH, MmGetPhysicalAddress((PVOID)0xC0603000).HighPart);

        //_SetCr3(dw32Cr3);
        break;
    }

    return;
}

static void _MoveFromCr(PEXITREASON_CRACCESS pstExitReasonCrAccess)
{
    switch (pstExitReasonCrAccess->CRX) {
    case 3:
        *((DWORD32*)&g_GuestRegs + pstExitReasonCrAccess->Reg) = _vmread(GUEST_CR3);
        break;
    }

    return;
}

//这里是测试设置0xE号为可执行，但不可读写（读写的数据会变成别的内存地址）
extern DWORD64 g_dw64FakePhysicalAddress;
extern DWORD64 g_dw64HookPhysicalAddress;
static void _HandleEPTViolation()
{
    EXITREASON_EPTVIOLATIONS stExitReasonEPTViolations;

    stExitReasonEPTViolations.QuadPart = _vmread(EXIT_QUALIFICATION);
    if (stExitReasonEPTViolations.isExecuteViolation == 1) {
        //0x0是0xE号中断的物理地址，可使用!vtop命令得到
        *(DWORD64*)g_dw64HookPhysicalAddress = 0x4e1000 | 0x34;
    }
    else if (stExitReasonEPTViolations.isReadViolation == 1 || stExitReasonEPTViolations.isWriteViolation == 1) {
        *(DWORD64*)g_dw64HookPhysicalAddress = g_dw64FakePhysicalAddress | 0x33;
    }
}