#ifndef __HOSTHANDLER_H__
#define __HOSTHANDLER_H__

#include <ntddk.h>
#include "Vmx.h"
#include "Register.h"

#ifdef __cplusplus
extern "C" {
#endif

    void _HostHander();

    static void _VMMEntryPointEbd();
    static void _HandleCpuid();
    static void _HandleCrAccess();
    static void _HandleVmcall();
    static void _MoveToCr(PEXITREASON_CRACCESS pstExitReasonCrAccess);
    static void _MoveFromCr(PEXITREASON_CRACCESS pstExitReasonCrAccess);
    static void _HandleEPTViolation();

#ifdef __cplusplus
}
#endif

#endif // !