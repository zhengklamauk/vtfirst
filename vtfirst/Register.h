#ifndef __REGISTER_H__
#define __REGISTER_H__

#include<ntddk.h>

#ifdef __cplusplus
extern "C" {
#endif


	DWORD32 GetCr4();
	VOID SetCr4(DWORD32 dw32Value_);

	DWORD32 GetEflags();
	VOID SetEflags(DWORD32 dw32Value_);


#ifdef __cplusplus
}
#endif

#endif // !__REGISTER_H__
