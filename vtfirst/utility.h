#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <ntddk.h>

void _KdPrint(char* Msg_)
{
	KdPrint(("[%s] [%s]: %s", __FILE__, __LINE__, Msg_));
}

#endif // !__UTILITY_H__
