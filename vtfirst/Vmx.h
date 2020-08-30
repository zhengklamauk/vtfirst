#ifndef __VMX_H__
#define __VMX_H__

#include <ntddk.h>

typedef struct _CPUIDRESULT {
    DWORD32 eax_;
    DWORD32 ebx_;
    DWORD32 ecx_;
    DWORD32 edx_;
}CPUIDRESULT, *PCPUIDRESULT;

//1查询cpuid的结构体
typedef struct _CPUID01 {
    DWORD32 eax;
    DWORD32 ebx;
    
    struct
    {
        unsigned SSE3 : 1;
        unsigned PCLMULQDQ : 1;
        unsigned DTES64 : 1;
        unsigned MONITOR : 1;
        unsigned DS_CPL : 1;
        unsigned VMX : 1;
        unsigned SMX : 1;
        unsigned EIST : 1;
        unsigned TM2 : 1;
        unsigned SSSE3 : 1;
        unsigned Reserved : 22;
    }ecx_;

    DWORD32 edx;
}CPUID01, *PCPUID01;

#ifdef __cplusplus
extern "C" {
#endif

	void _vmxon(DWORD32 dw32LowPart_, DWORD32 dw32HighPart_);
	void _vmxoff();
    void _cpuid(DWORD32 dw32Value_, PCPUIDRESULT pstCpuidResult_);

#ifdef __cplusplus
}
#endif

#endif // !__VMX_H__
