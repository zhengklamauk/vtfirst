#include "VtSystem.h"
#include "Register.h"
#include "utility.h"
#include "Vmx.h"

VMXREGION g_VmxRegion;  //��������VMX������ص���Ϣ


/**************************************************************************************************
 * ���ܣ�  ����VMX
 * ������
 * ���أ�  �ɹ�����STATUS_SUCCESS��ʧ�ܷ���STATUS_UNSUCCESSFUL
 * ������
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

	if (!_CreateVMXRegion(&g_VmxRegion)) {
		return STATUS_UNSUCCESSFUL;
	}

	_vmxon(g_VmxRegion.pvPhysicalAddress.LowPart, g_VmxRegion.pvPhysicalAddress.HighPart);

	_EFLAGS stEflags = { 0 };
	*(DWORD32*)&stEflags = _GetEflags();
	if (stEflags.CF != 0) {
		_KdPrint(("vmxon error\n"));

		if (g_VmxRegion.pvRegion != NULL) {
			ExFreePool(g_VmxRegion.pvRegion);
			g_VmxRegion.pvRegion = NULL;
			g_VmxRegion.pvPhysicalAddress.QuadPart = NULL;
		}

        *(DWORD32*)&stCr4 = _GetCr4();
        stCr4.VMXE = 0;
        _SetCr4(*(DWORD32*)&stCr4);

        return STATUS_UNSUCCESSFUL;
	}
	g_VmxRegion.isVMXON = TRUE;
	KdPrint(("VMX ON\n"));

	return STATUS_SUCCESS;
}


/**************************************************************************************************
 * ���ܣ�  �ر�VMX
 * ������
 * ���أ�  
 * ������
***************************************************************************************************/
VOID _StopVirtualTechnology()
{
	if (g_VmxRegion.isVMXON == TRUE) {
		_vmxoff();
		g_VmxRegion.isVMXON = FALSE;
	}

	_CR4 stCr4 = { 0 };
	*(DWORD32*)&stCr4 = _GetCr4();
	stCr4.VMXE = 0;
	_SetCr4(*(DWORD32*)&stCr4);

	if (g_VmxRegion.pvRegion != NULL) {
		ExFreePool(g_VmxRegion.pvRegion);
		g_VmxRegion.pvRegion = NULL;
		g_VmxRegion.pvPhysicalAddress.QuadPart = NULL;
	}

	KdPrint(("VMX OFF\n"));
    return;
}

/**************************************************************************************************
 * ���ܣ�  �ж�ϵͳ�Ƿ�֧��VMX
 * ������  
 * ���أ�  ֧��VMX�򷵻�TRUE�����򷵻�FALSE
 * ������
***************************************************************************************************/
static BOOLEAN _IsSupportVMX()
{
    //cpuid�ж�
    CPUIDRESULT stCpuidResult;
    _cpuid(1, &stCpuidResult);
    PCPUID01 pstCpuid01 = (PCPUID01)&stCpuidResult;
    if (pstCpuid01->ecx_.VMX != 1) {
        _KdPrint(("This CPU isn't support VMX\n"));
        return FALSE;
    }

    //IA32_FEATURE_CONTROL
    IA32_FEATURE_CONTROL_MSR stTmp;
    *(DWORD64*)&stTmp = _ReadMsr(MSR_IA32_FEATURE_CONTROL);
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

    //Cr4�����Cr4��ȷ��VMXģʽ��ֻ�����vt����������
    _CR4 stCr4;
    *(DWORD32*)&stCr4 = _GetCr4();
    if (stCr4.VMXE == 1) {
        _KdPrint(("VMXE is alreadly open\n"));
        return FALSE;
    }

	return TRUE;
}


/**************************************************************************************************
 * ���ܣ�  ����һ��4kb��VMX����
 * ������  VMXREGION�ṹָ�룬��������VMX����������ַ�������ַ��
 * ���أ�  ����VMX����ɹ�ʱ����TRUE��ʧ�ܷ���FALSE
 * ������
***************************************************************************************************/
static BOOLEAN _CreateVMXRegion(PVMXREGION pstVMXRegion_)
{
    pstVMXRegion_->pvRegion = ExAllocatePoolWithTag(NonPagedPool, 0x1000, 'VT01');
    if (pstVMXRegion_->pvRegion == NULL) {
        _KdPrint(("ExAllocatePoolWithTag\n"));
        pstVMXRegion_->pvPhysicalAddress.LowPart = 0;
        pstVMXRegion_->pvPhysicalAddress.HighPart = 0;
        return FALSE;
    }
    RtlZeroMemory(pstVMXRegion_->pvRegion, 0x1000);
    pstVMXRegion_->pvPhysicalAddress = MmGetPhysicalAddress(pstVMXRegion_->pvRegion);

	_Break3();
    VMX_BASIC_MSR stTmp;
    *(DWORD64*)&stTmp = _ReadMsr(MSR_IA32_VMX_BASIC);
    DWORD32* dw32Version = (DWORD32*)pstVMXRegion_->pvRegion;
    *dw32Version = stTmp.RevId;

    return TRUE;
}