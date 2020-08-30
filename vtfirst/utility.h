#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <ntddk.h>

#define _KdPrint(Msg_) KdPrint(("[%s] [%d]: %s", __FILE__, __LINE__, (Msg_)))
#define _Break3() {__asm int 3}

#endif // !__UTILITY_H__
