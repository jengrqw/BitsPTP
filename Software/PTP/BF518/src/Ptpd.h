/* =============================================================================
 *
 *  Description: This is a C++ to C Thread Header file for Thread Ptpd
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

#ifndef _Ptpd_H_
#define _Ptpd_H_

#ifndef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress: 177,401,451,826,831,1462)
#endif



#include "VDK.h"
#ifndef _MISRA_RULES
#pragma diag(pop)
#endif
    
#include "constants.h"
#include "dep/constants_dep.h"
#include "dep/datatypes_dep.h"
#include "datatypes.h"
#include "dep/ptpd_dep.h"
#include "cdefbf518.h"
#include "debug.h"

   

#ifdef __ECC__	/* for C/C++ access */
#ifdef __cplusplus
extern "C" void Ptpd_InitFunction(void**, VDK::Thread::ThreadCreationBlock const *);
#else
extern "C" void Ptpd_InitFunction(void** inPtr, VDK_ThreadCreationBlock const * pTCB);
#endif
extern "C" void Ptpd_DestroyFunction(void** inPtr);
extern "C" int  Ptpd_ErrorFunction(void** inPtr);
extern "C" void Ptpd_RunFunction(void** inPtr);
#endif /* __ECC__ */

#ifdef __cplusplus
#include <new>


class Ptpd_Wrapper : public VDK::Thread
{
public:
    Ptpd_Wrapper(VDK::ThreadCreationBlock &t)
        : VDK::Thread(t)
    { Ptpd_InitFunction(&m_DataPtr, &t); }

    ~Ptpd_Wrapper()
    { Ptpd_DestroyFunction(&m_DataPtr); }

    int ErrorHandler()
    {
      return Ptpd_ErrorFunction(&m_DataPtr);
     }

    void Run()
    { Ptpd_RunFunction(&m_DataPtr); }

    static VDK::Thread* Create(VDK::Thread::ThreadCreationBlock &t)
    { return new (t) Ptpd_Wrapper(t); }
};

#endif /* __cplusplus */
#endif /* _Ptpd_H_ */

/* ========================================================================== */
