#ifndef __REGISTER_H__
#define __REGISTER_H__

#include<ntddk.h>

#define IA32_SYSENTER_CS                0x174
#define IA32_SYSENTER_ESP               0x175
#define IA32_SYSENTER_EIP               0x176
#define IA32_VMX_PINBASED_CTLS          0x481
#define IA32_VMX_PROCBASED_CTLS         0x482
#define IA32_VMX_EXIT_CTLS              0x483
#define IA32_VMX_ENTRY_CTLS             0x484

#define IA32_FEATURE_CONTROL 		    0x03a
typedef struct _MSR_IA32_FEATURE_CONTROL_MSR
{
    unsigned Lock : 1;		    // Bit 0 is the lock bit - cannot be modified once lock is set
    unsigned Reserved1 : 1;		// Undefined
    unsigned EnableVmxon : 1;	// Bit 2. If this bit is clear, VMXON causes a general protection exception
    unsigned Reserved2 : 29;	// Undefined
    unsigned Reserved3 : 32;	// Undefined

} MSR_IA32_FEATURE_CONTROL, *PMSR_IA32_FEATURE_CONTROL;

#define IA32_VMX_BASIC                  0x480
typedef struct _MSR_IA32_VMX_BASIC
{
    unsigned RevId : 32;
    unsigned szVmxOnRegion : 12;
    unsigned ClearBit : 1;
    unsigned Reserved : 3;
    unsigned PhysicalWidth : 1;
    unsigned DualMonitor : 1;
    unsigned MemoryType : 4;
    unsigned VmExitInformation : 1;
    unsigned Reserved2 : 9;
} MSR_IA32_VMX_BASIC, *PMSR_IA32VMX_BASIC;

typedef struct {
    unsigned int PE : 1;
    unsigned int MP : 1;
    unsigned int EM : 1;
    unsigned int TS : 1;
    unsigned int ET : 1;
    unsigned int NE : 1;
    unsigned int Reserved_1 : 10;   //bit 6
    unsigned int WP : 1;            //bit 16
    unsigned int Reserved_2 : 1;
    unsigned int AM : 1;
    unsigned int Reserved_3 : 10;   //bit 19
    unsigned int NW : 1;            //bit 29
    unsigned int CD : 1;
    unsigned int PG : 1;
}_CR0;

typedef struct {
    unsigned int VME : 1;
    unsigned int PVI : 1;
    unsigned int TSD : 1;
    unsigned int DE : 1;
    unsigned int PSE : 1;
    unsigned int PAE : 1;
    unsigned int MCE : 1;
    unsigned int PGE : 1;			//bit 7
    unsigned int PCE : 1;
    unsigned int OSFXSR : 1;
    unsigned int PSXMMEXCPT : 1;
    unsigned int UNKONOWN_1 : 1;	//zero
    unsigned int UNKONOWN_2 : 1;	//zero
    unsigned int VMXE : 1;			//bit 13
    unsigned int Reserved : 18;
    //unsigned int Reserved_64 : 32;
}_CR4;

typedef struct {
    unsigned int CF : 1;
    unsigned int Unknown_1 : 1;
    unsigned int PF : 1;
    unsigned int Unknown_2 : 1;
    unsigned int AF : 1;
    unsigned int Unknown_3 : 1;
    unsigned int ZF : 1;
    unsigned int SF : 1;			//bit 7
    unsigned int TF : 1;
    unsigned int IF : 1;
    unsigned int DF : 1;
    unsigned int OF : 1;
    unsigned int IOPL : 2;
    unsigned int NT : 1;
    unsigned int Unknown_4 : 1;		//bit 15
    unsigned int RF : 1;
    unsigned int VM : 1;
    unsigned int AC : 1;
    unsigned int VIF : 1;
    unsigned int VIP : 1;
    unsigned int ID : 1;			//bit 21
    unsigned int Reserved : 10;
    //unsigned Reserved_64 : 32;
}_EFLAGS;

#ifdef __cplusplus
extern "C" {
#endif

    DWORD32 _GetCr0();
    VOID _SetCr0(DWORD32 dw32Value_);

    DWORD32 _GetCr3();
    VOID _SetCr3(DWORD32 dw32Value_);

	DWORD32 _GetCr4();
	VOID _SetCr4(DWORD32 dw32Value_);

	DWORD32 _GetEflags();
	VOID _SetEflags(DWORD32 dw32Value_);

    DWORD32 _GetCs();
    DWORD32 _GetSs();
    DWORD32 _GetDs();
    DWORD32 _GetEs();
    DWORD32 _GetFs();
    DWORD32 _GetGs();

    DWORD32 _GetDescriptorBaseBySelector(DWORD32 dw32Selector_);
    DWORD32 _GetDescriptorLimitBySelector(DWORD32 dw32Selector_);

    DWORD32 _GetTr();
    DWORD32 _GetTrBase();

    DWORD32 _GetLdt();
    DWORD64 _GetGdt();
    DWORD64 _GetIdt();

    DWORD64 _ReadMsr(DWORD32 dw32Index_);

#ifdef __cplusplus
}
#endif

#endif // !__REGISTER_H__
