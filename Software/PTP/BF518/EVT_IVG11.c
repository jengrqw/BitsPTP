/* =============================================================================
 *
 *   Description: This is the Interrupt Service Routine called EVT_IVG11_Entry
 *
 * -----------------------------------------------------------------------------
 *  Comments: 
 *
 * ===========================================================================*/

#include "VDK.h"

#include <sys/exception.h>
#pragma file_attr("prefersMemNum=30")
#pragma file_attr("prefersMem=internal")
#pragma file_attr("ISR")

/* define VDK_REENTRANT_ISR macro if nested interrupts are allowed. This 
   definition can be done in a global project option if all interrupts allow
   nesting or in a particular interrupt file if there are any interrupts
   in the application that don't allow nesting */ 

#ifdef VDK_REENTRANT_ISR
EX_REENTRANT_HANDLER(EVT_IVG11_Entry)
#else
EX_INTERRUPT_HANDLER(EVT_IVG11_Entry) 
#endif
{
/* Insert your ISR code here */
}

/* ========================================================================== */
