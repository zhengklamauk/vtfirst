#ifndef __HOSTHANDLER_H__
#define __HOSTHANDLER_H__

#include <ntddk.h>

#ifdef __cplusplus
extern "C" {
#endif

    void _HostHander();

    static void _VMMEntryPointEbd();

#ifdef __cplusplus
}
#endif

#endif // !