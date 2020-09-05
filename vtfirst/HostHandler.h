#ifndef __HOSTHANDLER_H__
#define __HOSTHANDLER_H__

#include <ntddk.h>
#include "Vmx.h"

#ifdef __cplusplus
extern "C" {
#endif

    void _HostHander();

    static void _VMMEntryPointEbd();
    static void _HandleCpuid();
    static void _HandleCrAccess();
    static void _HandleVmcall();
    static void _MoveToCr(PEXITREASONCRACCESS pstExitReasonCrAccess);
    static void _MoveFromCr(PEXITREASONCRACCESS pstExitReasonCrAccess);

#ifdef __cplusplus
}
#endif

#endif // !