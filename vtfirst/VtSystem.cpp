#include "VtSystem.h"
#include "Register.h"
#include "utility.h"

VMXINFORMATION g_VMXInformation;  //用来保存VMX区域相关的信息
BOOLEAN g_isStopVMX = FALSE;
VMXSTOP g_VMXStop;

/**************************************************************************************************
 * 功能：  开启VMX
 * 参数：  
 * 返回：  成功返回STATUS_SUCCESS，失败返回STATUS_UNSUCCESSFUL
 * 其他：
***************************************************************************************************/
NTSTATUS _StartVirtualTechnology()
{
	if (!_IsSupportVMX()) {
		return STATUS_UNSUCCESSFUL;
	}

	_CR4 stCr4 = { 0 };
	*(DWORD32*)&stCr4 = _GetCr4();
	stCr4.VMXE = 1;
	_SetCr4(*(DWORD32*)&stCr4);

	if (!_CreateHostRegion(&g_VMXInformation)) {
		return STATUS_UNSUCCESSFUL;
	}

	_vmxon(g_VMXInformation.pvHostPhysicalAddress.LowPart, g_VMXInformation.pvHostPhysicalAddress.HighPart);

	_EFLAGS stEflags = { 0 };
	*(DWORD32*)&stEflags = _GetEflags();
	if (stEflags.CF != 0) {
		_KdPrint(("vmxon error\n"));

        _FreeVMXInformationMember(&g_VMXInformation);

        *(DWORD32*)&stCr4 = _GetCr4();
        stCr4.VMXE = 0;
        _SetCr4(*(DWORD32*)&stCr4);

        return STATUS_UNSUCCESSFUL;
	}
    g_VMXInformation.isVMXON = TRUE;
	KdPrint(("VMX ON\n"));

	return STATUS_SUCCESS;
}


/**************************************************************************************************
 * 功能：  启动GUEST
 * 参数：  arg0-HOST处理函数
          arg1-GUSET入口函数
 * 返回：
 * 其他：
***************************************************************************************************/
BOOLEAN _LaunchGuest(PVOID pvHostHandler_, PVOID pvGuestEntry_)
{
     if (!_CreateVMCSRegion(&g_VMXInformation)) {
        return STATUS_UNSUCCESSFUL;
     }

     _vmclear(g_VMXInformation.pvVMCSPhysicalAddress.LowPart, g_VMXInformation.pvVMCSPhysicalAddress.HighPart);
     _EFLAGS stEflags = { 0 };
     *(DWORD32*)&stEflags = _GetEflags();
     if (stEflags.CF != 0 || stEflags.ZF != 0) {
         _KdPrint("_vmclear error\n");
         return STATUS_UNSUCCESSFUL;
     }
     _vmptrld(g_VMXInformation.pvVMCSPhysicalAddress.LowPart, g_VMXInformation.pvVMCSPhysicalAddress.HighPart);

     _SetupVMCS(pvHostHandler_, pvGuestEntry_, &g_VMXInformation);

     _vmlaunch();

     return STATUS_SUCCESS;
}


/**************************************************************************************************
 * 功能：  关闭VMX
 * 参数：
 * 返回：  
 * 其他：
***************************************************************************************************/
VOID _StopVirtualTechnology()
{
	if (g_VMXInformation.isVMXON == TRUE) {
        g_VMXInformation.isVMXON = FALSE;
        g_isStopVMX = TRUE;

        __asm {
            pushad;
            pushfd;
            
            mov g_VMXStop.ReturnEip, offset __VMXSTOP;
            mov g_VMXStop.ReturnEsp, esp;
        }
        _vmcall();
        __asm {
        __VMXSTOP:
            popfd;
            popad;
        }
	}

    _SetGdt((DWORD32)&g_VMXInformation.liGdt);
    _SetIdt((DWORD32)&g_VMXInformation.liIdt);
    if (g_VMXInformation.isEnableEPT == TRUE) {
        _FreeEPTTable(&g_VMXInformation);
    }
    
	_CR4 stCr4 = { 0 };
	*(DWORD32*)&stCr4 = _GetCr4();
	stCr4.VMXE = 0;
	_SetCr4(*(DWORD32*)&stCr4);

    _FreeVMXInformationMember(&g_VMXInformation);

	KdPrint(("VMX OFF\n"));
    return;
}


/**************************************************************************************************
 * 功能：  判断系统是否支持VMX
 * 参数：  
 * 返回：  支持VMX则返回TRUE，否则返回FALSE
 * 其他：
***************************************************************************************************/
static BOOLEAN _IsSupportVMX()
{
    //cpuid判断
    CPUIDRESULT stCpuidResult;
    _cpuid(1, &stCpuidResult);
    PCPUID01 pstCpuid01 = (PCPUID01)&stCpuidResult;
    if (pstCpuid01->ecx_.VMX != 1) {
        _KdPrint(("This CPU isn't support VMX\n"));
        return FALSE;
    }

    //IA32_FEATURE_CONTROL
    MSR_IA32_FEATURE_CONTROL stTmp;
    *(DWORD64*)&stTmp = _ReadMsr(IA32_FEATURE_CONTROL);
    if (stTmp.Lock != 1) {
        _KdPrint(("VMX is unlock\n"));
        return FALSE;
    }

    //Cr0
    _CR0 stCr0;
    *(DWORD32*)&stCr0 = _GetCr0();
    if (stCr0.PE != 1 || stCr0.PG != 1 || stCr0.NE != 1) {
        _KdPrint(("Cr0 error\n"));
        return FALSE;
    }

    //Cr4，检查Cr4是确保VMX模式下只有这个vt驱动在运行
    _CR4 stCr4;
    *(DWORD32*)&stCr4 = _GetCr4();
    if (stCr4.VMXE == 1) {
        _KdPrint(("VMXE is alreadly open\n"));
        return FALSE;
    }

	return TRUE;
}


/**************************************************************************************************
 * 功能：  创建一个4kb的HOST区域
 * 参数：  VMXINFORMATION结构指针，用来保存VMX区域的虚拟地址和物理地址等
 * 返回：  创建VMX区域成功时返回TRUE，失败返回FALSE
 * 其他：
***************************************************************************************************/
static BOOLEAN _CreateHostRegion(PVMXINFORMATION pstVMXInformation_)
{
    pstVMXInformation_->pvHostAddress = ExAllocatePoolWithTag(NonPagedPool, 0x1000, 'CVI1');
    if (pstVMXInformation_->pvHostAddress == NULL) {
        _KdPrint(("ExAllocatePoolWithTag\n"));
        pstVMXInformation_->pvHostPhysicalAddress.LowPart = 0;
        pstVMXInformation_->pvHostPhysicalAddress.HighPart = 0;
        return FALSE;
    }
    RtlZeroMemory(pstVMXInformation_->pvHostAddress, 0x1000);
    pstVMXInformation_->pvHostPhysicalAddress = MmGetPhysicalAddress(pstVMXInformation_->pvHostAddress);

    MSR_IA32_VMX_BASIC stTmp;
    *(DWORD64*)&stTmp = _ReadMsr(IA32_VMX_BASIC);
    DWORD32* dw32Version = (DWORD32*)pstVMXInformation_->pvHostAddress;
    *dw32Version = stTmp.RevId;

    pstVMXInformation_->pvHostStack = ExAllocatePoolWithTag(NonPagedPool, HOST_STACK_SIZE, 'CVI2');
    if (pstVMXInformation_->pvHostStack == NULL) {
        _KdPrint("ExAllocatePoolWithTag error\n");
        ExFreePool(pstVMXInformation_->pvHostAddress);
        pstVMXInformation_->pvHostAddress = NULL;
        pstVMXInformation_->pvHostPhysicalAddress.QuadPart = NULL;
        return FALSE;
    }

    return TRUE;
}


/**************************************************************************************************
 * 功能：  创建一个VMCS结构区域
 * 参数：  保存一信息的结构体
 * 返回：  成功返回TRUE，失败返回FALSE
 * 其他：
***************************************************************************************************/
static BOOLEAN _CreateVMCSRegion(PVMXINFORMATION pstVMXInformation_)
{
    pstVMXInformation_->pvVMCSAddress= ExAllocatePoolWithTag(NonPagedPool, 0x1000, 'CVR1');
    if (pstVMXInformation_->pvVMCSAddress == NULL) {
        _KdPrint(("ExAllocatePoolWithTag\n"));
        pstVMXInformation_->pvVMCSPhysicalAddress.LowPart = 0;
        pstVMXInformation_->pvVMCSPhysicalAddress.HighPart = 0;
        return FALSE;
    }
    RtlZeroMemory(pstVMXInformation_->pvVMCSAddress, 0x1000);
    pstVMXInformation_->pvVMCSPhysicalAddress = MmGetPhysicalAddress(pstVMXInformation_->pvVMCSAddress);

    MSR_IA32_VMX_BASIC stTmp;
    *(DWORD64*)&stTmp = _ReadMsr(IA32_VMX_BASIC);
    DWORD32* dw32Version = (DWORD32*)pstVMXInformation_->pvVMCSAddress;
    *dw32Version = stTmp.RevId;

    pstVMXInformation_->pvGuestStack = ExAllocatePoolWithTag(NonPagedPool, GUEST_STACK_SIZE, 'CVR2');
    if (pstVMXInformation_->pvGuestStack == NULL) {
        _KdPrint("ExAllocatePoolWithTag error\n");
        ExFreePool(pstVMXInformation_->pvVMCSAddress);
        pstVMXInformation_->pvVMCSAddress = NULL;
        pstVMXInformation_->pvVMCSPhysicalAddress.QuadPart = NULL;
        return FALSE;
    }

    return TRUE;
}


/**************************************************************************************************
 * 功能：  释放VMXINFORMATION结构中的内存块
 * 参数：  VMXINFORMATION结构变量的地址
 * 返回：
 * 其他：
***************************************************************************************************/
static void _FreeVMXInformationMember(PVMXINFORMATION pstVMXInformation_)
{
    if (pstVMXInformation_->pvHostAddress != NULL) {
        ExFreePool(pstVMXInformation_->pvHostAddress);
        pstVMXInformation_->pvHostAddress = NULL;
        pstVMXInformation_->pvHostPhysicalAddress.QuadPart = NULL;
    }
    if (pstVMXInformation_->pvVMCSAddress != NULL) {
        ExFreePool(pstVMXInformation_->pvVMCSAddress);
        pstVMXInformation_->pvVMCSAddress = NULL;
        pstVMXInformation_->pvVMCSPhysicalAddress.QuadPart = NULL;
    }
    if (pstVMXInformation_->pvHostStack != NULL) {
        ExFreePool(pstVMXInformation_->pvHostStack);
        pstVMXInformation_->pvHostStack = NULL;
    }
    if (pstVMXInformation_->pvGuestStack != NULL) {
        ExFreePool(pstVMXInformation_->pvGuestStack);
        pstVMXInformation_->pvGuestStack = NULL;
    }

    return;
}


/**************************************************************************************************
 * 功能：  设置VMCS结构的值
 * 参数：  arg0-Host处理函数
          arg1-Guest入口函数
          arg2-VMXINFORMATION结构体变量
 * 返回：  成功返回TRUE，失败返回FALSE
 * 其他：
***************************************************************************************************/
void _SetupVMCS(PVOID pvHostHandler_, PVOID pvGuestEntry_, PVMXINFORMATION pstVMXInformation_)
{
    //GUEST-STATE AREA
    _vmwrite(GUEST_CR0, _GetCr0());
    _vmwrite(GUEST_CR3, _GetCr3());
    _vmwrite(GUEST_CR4, _GetCr4());
    _vmwrite(GUEST_DR7, 0x400);
    _vmwrite(GUEST_RFLAGS, _GetEflags() & ~0x200);

    _vmwrite(GUEST_CS_SELECTOR, _GetCs() & 0xFFF8);
    _vmwrite(GUEST_SS_SELECTOR, _GetSs() & 0xFFF8);
    _vmwrite(GUEST_DS_SELECTOR, _GetDs() & 0xFFF8);
    _vmwrite(GUEST_ES_SELECTOR, _GetEs() & 0xFFF8);
    _vmwrite(GUEST_FS_SELECTOR, _GetFs() & 0xFFF8);
    _vmwrite(GUEST_GS_SELECTOR, _GetGs() & 0xFFF8);
    _vmwrite(GUEST_TR_SELECTOR, _GetTr() & 0xFFF8);

    _vmwrite(GUEST_LDTR_AR_BYTES, 0x10000);

    _vmwrite(GUEST_SS_AR_BYTES, 0x10000);
    _vmwrite(GUEST_DS_AR_BYTES, 0x10000);
    _vmwrite(GUEST_ES_AR_BYTES, 0x10000);
    _vmwrite(GUEST_FS_AR_BYTES, 0x10000);
    _vmwrite(GUEST_GS_AR_BYTES, 0x10000);

    _vmwrite(GUEST_CS_AR_BYTES, 0xC09B);
    _vmwrite(GUEST_CS_BASE, 0);
    _vmwrite(GUEST_CS_LIMIT, 0xFFFFFFFF);

    _vmwrite(GUEST_TR_AR_BYTES, 0x008B);
    _vmwrite(GUEST_TR_BASE, _GetDescriptorBaseBySelector(_GetTr()));
    _vmwrite(GUEST_TR_LIMIT, _GetDescriptorLimitBySelector(_GetTr()));

    LARGE_INTEGER laTmp;
    laTmp.QuadPart = _GetGdt();
    _vmwrite(GUEST_GDTR_BASE, laTmp.LowPart);
    _vmwrite(GUEST_GDTR_LIMIT, laTmp.HighPart);
    laTmp.QuadPart = _GetIdt();
    _vmwrite(GUEST_IDTR_BASE, laTmp.LowPart);
    _vmwrite(GUEST_IDTR_LIMIT, laTmp.HighPart);

    _vmwrite(GUEST_SYSENTER_CS, _ReadMsr(IA32_SYSENTER_CS));
    _vmwrite(GUEST_SYSENTER_ESP, _ReadMsr(IA32_SYSENTER_ESP));
    _vmwrite(GUEST_SYSENTER_EIP, _ReadMsr(IA32_SYSENTER_EIP));

    _vmwrite(GUEST_RSP, (DWORD32)pstVMXInformation_->pvGuestStack + GUEST_STACK_SIZE);
    _vmwrite(GUEST_RIP, (DWORD32)pvGuestEntry_);

    _vmwrite(VMCS_LINK_POINTER, 0xFFFFFFFF);
    _vmwrite(VMCS_LINK_POINTER_HIGH, 0xFFFFFFFF);

    //HOST-STATE AREA
    _vmwrite(HOST_CR0, _GetCr0());
    _vmwrite(HOST_CR3, _GetCr3());
    _vmwrite(HOST_CR4, _GetCr4());

    _vmwrite(HOST_CS_SELECTOR, _GetCs() & 0xFFF8);
    _vmwrite(HOST_SS_SELECTOR, _GetSs() & 0xFFF8);
    _vmwrite(HOST_DS_SELECTOR, _GetDs() & 0xFFF8);
    _vmwrite(HOST_ES_SELECTOR, _GetEs() & 0xFFF8);
    _vmwrite(HOST_FS_SELECTOR, _GetFs() & 0xFFF8);
    _vmwrite(HOST_GS_SELECTOR, _GetGs() & 0xFFF8);

    _vmwrite(HOST_FS_BASE, _GetDescriptorBaseBySelector(_GetFs()));
    _vmwrite(HOST_GS_BASE, _GetDescriptorBaseBySelector(_GetGs()));

    _vmwrite(HOST_TR_SELECTOR, _GetTr() & 0xFFF8);
    _vmwrite(HOST_TR_BASE, _GetDescriptorBaseBySelector(_GetTr()));

    laTmp.QuadPart = _GetGdt();
    _vmwrite(HOST_GDTR_BASE, laTmp.LowPart);
    pstVMXInformation_->liGdt.QuadPart = laTmp.QuadPart;
    laTmp.QuadPart = _GetIdt();
    _vmwrite(HOST_IDTR_BASE, laTmp.LowPart);
    pstVMXInformation_->liIdt.QuadPart = laTmp.QuadPart;

    _vmwrite(HOST_IA32_SYSENTER_CS, _ReadMsr(IA32_SYSENTER_CS));
    _vmwrite(HOST_IA32_SYSENTER_ESP, _ReadMsr(IA32_SYSENTER_ESP));
    _vmwrite(HOST_IA32_SYSENTER_EIP, _ReadMsr(IA32_SYSENTER_EIP));

    _vmwrite(HOST_RSP, (DWORD32)pstVMXInformation_->pvHostStack + HOST_STACK_SIZE);
    _vmwrite(HOST_RIP, (DWORD32)pvHostHandler_);   //这里要修改为_HostHandler

    //VM-EXECUTION CONTROL FIELDS
    _vmwrite(PIN_BASED_VM_EXEC_CONTROL, _AdjustControlValue(0, IA32_VMX_PINBASED_CTLS));
    _vmwrite(CPU_BASED_VM_EXEC_CONTROL, _AdjustControlValue(0, IA32_VMX_PROCBASED_CTLS));

    //VM-EXIT CONTROL FIELDS
    _vmwrite(VM_EXIT_CONTROLS, _AdjustControlValue(0, IA32_VMX_EXIT_CTLS));
    //VM-ENTRY CONTROL FIELDS
    _vmwrite(VM_ENTRY_CONTROLS, _AdjustControlValue(0, IA32_VMX_ENTRY_CTLS));

    //------------------------------以上为最小的设置------------------------------

    //开启EPT
    if (pstVMXInformation_->isEnableEPT == TRUE) {
        
        _vmwrite(CPU_BASED_VM_EXEC_CONTROL, _AdjustControlValue(0x80000000, IA32_VMX_PROCBASED_CTLS));
        _vmwrite(SECONDARY_VM_EXEC_CONTROL, _AdjustControlValue(0x2, IA32_VMX_PROCBASED_CTLS2));
        pstVMXInformation_->pvPML4TPhysicalAddress = MmGetPhysicalAddress(pstVMXInformation_->pv64PML4T);

        DWORD32 dw32Type;
LARGE_INTEGER liTmp;
liTmp.QuadPart = _ReadMsr(IA32_VMX_EPT_VPID_CAP);
if ((liTmp.LowPart & 0x40) == 0) {
    dw32Type = 0;
}
else {
    dw32Type = 6;
}

_vmwrite(EPT_POINTER, (pstVMXInformation_->pvPML4TPhysicalAddress.LowPart | dw32Type | (3 << 3)) & 0xFFFFFFFF);
_vmwrite(EPT_POINTER_HIGH, pstVMXInformation_->pvPML4TPhysicalAddress.HighPart);
_ForPAE();
    }

    return;
}


/**************************************************************************************************
 * 功能：  调整控制域的值
 * 参数：  arg0-原始值
          arg1-要查询的MSR寄存器的索引值
 * 返回：  调整后的值
 * 其他：
***************************************************************************************************/
DWORD32 _AdjustControlValue(DWORD32 dw32Original_, DWORD32 dw32MsrIndex_)
{
    LARGE_INTEGER liMsrValue;
    liMsrValue.QuadPart = _ReadMsr(dw32MsrIndex_);
    dw32Original_ |= liMsrValue.LowPart;
    dw32Original_ &= liMsrValue.HighPart;
    return dw32Original_;
}

//这两个全局变量是测试设置0xE号为可执行，但不可读写（读写的数据会变成别的内存地址）
DWORD64 g_dw64FakePhysicalAddress;  //读写0xE号中断时，实际会变成读写这里的空间
DWORD64 g_dw64HookPhysicalAddress;  
/**************************************************************************************************
 * 功能：  初始化页表
 * 参数：
 * 返回：  返回PML4T，返回0则表示失败
 * 其他：
***************************************************************************************************/
PVOID64 _SetupEPT()
{
    PVOID64 pvPML4T;
    PVOID64 pvPDPT;
    PHYSICAL_ADDRESS paPDPT;

    InitializeListHead(&g_VMXInformation.lsEPTTable);

    LARGE_INTEGER laCTLS2;
    laCTLS2.QuadPart = _ReadMsr(IA32_VMX_PROCBASED_CTLS2);
    if ((laCTLS2.HighPart & 2) == 0) {
        _KdPrint("Not Support EPT\n");
        return NULL;
    }

    //测试设置0xE号为可执行，但不可读写（读写的数据会变成别的内存地址）
    PVOID pvTmp = _AllocateOnePageSize(&g_VMXInformation);
    g_dw64FakePhysicalAddress = MmGetPhysicalAddress(pvTmp).QuadPart;

    //_Break3();
    //L1
    pvPML4T = _AllocateOnePageSize(&g_VMXInformation);
    if (pvPML4T == NULL) {
        _KdPrint("_AllocateOnePageSize error\n");
        return NULL;
    }
    //L2
    pvPDPT = _AllocateOnePageSize(&g_VMXInformation);
    if (pvPDPT == NULL) {
        _KdPrint("_AllocateOnePageSize error\n");
        return NULL;
    }
    paPDPT = MmGetPhysicalAddress(pvPDPT);
    *(DWORD64*)pvPML4T = (paPDPT.QuadPart) | 7;   //7 = P | R/W | U/S

    PVOID64 pvPDT;
    PHYSICAL_ADDRESS paPDT;
    PVOID64 pvPT;
    PHYSICAL_ADDRESS paPT;
    int i, j, k;
    for (i = 0; i < 4; i++) {   //L3
        pvPDT = _AllocateOnePageSize(&g_VMXInformation);
        if (pvPDT == NULL) {
            _KdPrint("_AllocateOnePageSize error\n");
            return NULL;
        }
        paPDT = MmGetPhysicalAddress(pvPDT);
        *(DWORD64*)pvPDPT = (paPDT.QuadPart) | 7;
        pvPDPT = (PVOID64)((DWORD64)pvPDPT + 8);

        for (j = 0; j < 512; j++) {     //L4
            pvPT = _AllocateOnePageSize(&g_VMXInformation);
            if (pvPT == NULL) {
                _KdPrint("_AllocateOnePageSize error\n");
                return NULL;
            }
            paPT = MmGetPhysicalAddress(pvPT);
            *(DWORD64*)pvPDT = (paPT.QuadPart) | 7;
            pvPDT = (PVOID64)((DWORD64)pvPDT + 8);

            for (k = 0; k < 512; k++) {     //physical address
                *(DWORD64*)pvPT = ((i << 30) | (j << 21) | (k << 12) | 0x37) & 0xFFFFFFFF;

                //这个条件块是测试设置0xE号为可执行，但不可读写（读写的数据会变成别的内存地址）
                //0x0是0xE号中断的物理地址，可使用!vtop命令得到
                if (0x4e1000 == (((i << 30) | (j << 21) | (k << 12)) & 0xFFFFFFFF)) {
                    *(DWORD64*)pvPT = ((i << 30) | (j << 21) | (k << 12) | 0x34) & 0xFFFFFFFF;
                    g_dw64HookPhysicalAddress = (DWORD64)pvPT;
                }

                pvPT = (PVOID64)((DWORD64)pvPT + 8);
            }
        }
    }

    g_VMXInformation.isEnableEPT = TRUE;
    g_VMXInformation.pv64PML4T = pvPML4T;

    return pvPML4T;
}


/**************************************************************************************************
 * 功能：  分配一个页大小的空间，并把这个地址挂到一个链表中以方便释放
 * 参数：  VMXINFORMATION结构体，内含链表成员
 * 返回：  返回分配的空间的地址，失败返回0
 * 其他：
***************************************************************************************************/
static PVOID64 _AllocateOnePageSize(PVMXINFORMATION pstVMXInformation_)
{
    PVOID pvPage;
    PEPTTABLE pstEPTTable;

    pvPage = ExAllocatePoolWithTag(NonPagedPool, 0x1000, 'AOP1');
    if (pvPage != NULL) {
        RtlZeroMemory(pvPage, 0x1000);
    }
    pstEPTTable = (PEPTTABLE)ExAllocatePoolWithTag(PagedPool, sizeof(EPTTABLE), 'AOP2');
    if (pstEPTTable == NULL) {
        if (pvPage != NULL) {
            ExFreePool(pvPage);
        }
    }
    else {
        pstEPTTable->pvTableAddress = pvPage;
        InsertTailList(&pstVMXInformation_->lsEPTTable, &pstEPTTable->lsNode);
    }

    return (PVOID64)pvPage;
}


/**************************************************************************************************
 * 功能：  释放页表所占用的内存
 * 参数：
 * 返回：
 * 其他：
***************************************************************************************************/
static void _FreeEPTTable(PVMXINFORMATION pstVMXInformation_)
{
    LIST_ENTRY* pleList;
    PEPTTABLE pstTable;
 
    while (IsListEmpty(&pstVMXInformation_->lsEPTTable) != TRUE) {
        pleList = RemoveHeadList(&pstVMXInformation_->lsEPTTable);
        if (pleList != &pstVMXInformation_->lsEPTTable) {
            pstTable = (PEPTTABLE)pleList;
            if (pstTable->pvTableAddress != NULL) {
                ExFreePool(pstTable->pvTableAddress);
            }
        }
    }
    return;
}


/**************************************************************************************************
 * 功能：  2-9-9-12分页开启EPT要做的处理
 * 参数：
 * 返回：
 * 其他：  此函数可能会过时
***************************************************************************************************/
static void _ForPAE()
{
    _vmwrite(GUEST_PDPTR0, MmGetPhysicalAddress((PVOID)0xC0600000).LowPart | 1);
    _vmwrite(GUEST_PDPTR0_HIGH, MmGetPhysicalAddress((PVOID)0xC0600000).HighPart);

    _vmwrite(GUEST_PDPTR1, MmGetPhysicalAddress((PVOID)0xC0601000).LowPart | 1);
    _vmwrite(GUEST_PDPTR1_HIGH, MmGetPhysicalAddress((PVOID)0xC0601000).HighPart);

    _vmwrite(GUEST_PDPTR2, MmGetPhysicalAddress((PVOID)0xC0602000).LowPart | 1);
    _vmwrite(GUEST_PDPTR2_HIGH, MmGetPhysicalAddress((PVOID)0xC0602000).HighPart);

    _vmwrite(GUEST_PDPTR3, MmGetPhysicalAddress((PVOID)0xC0603000).LowPart | 1);
    _vmwrite(GUEST_PDPTR3_HIGH, MmGetPhysicalAddress((PVOID)0xC0603000).HighPart);

    return;
}