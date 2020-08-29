#ifndef __VMX_H__
#define __VMX_H__

#include <ntddk.h>

#ifdef __cplusplus
extern "C" {
#endif


	void _vmxon(DWORD32 dw32LowPart_, DWORD32 dw32HighPart_);
	void _vmxoff();


#ifdef __cplusplus
}
#endif

#endif // !__VMX_H__
