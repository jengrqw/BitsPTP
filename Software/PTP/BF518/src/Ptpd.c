/* =============================================================================
 *
 *  Description: This is a C implementation for Thread Ptpd
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

/* Get access to any of the VDK features & datatypes used */
#include "Ptpd.h"

#pragma file_attr("OS_Component=Threads")
#pragma file_attr("Threads")

RunTimeOpts rtOpts;  /* statically allocated run-time configuration data */

extern char hwaddr[6];

void
Ptpd_RunFunction(void **inPtr)
{
    /* Put the thread's "main" Initialization HERE */

 // int argc;
 // char **argv;
  PtpClock *ptpClock;
  Integer16 ret;
  

  /* initialize run-time options to reasonable values */
  rtOpts.syncInterval = DEFUALT_SYNC_INTERVAL;
  memcpy(rtOpts.subdomainName, DEFAULT_PTP_DOMAIN_NAME, PTP_SUBDOMAIN_NAME_LENGTH);
  memcpy(rtOpts.clockIdentifier, IDENTIFIER_DFLT, PTP_CODE_STRING_LENGTH);
  rtOpts.clockVariance = (UInteger32)DEFAULT_CLOCK_VARIANCE;
  rtOpts.clockStratum = DEFAULT_CLOCK_STRATUM;
  rtOpts.unicastAddress[0] = 0;
  rtOpts.inboundLatency.nanoseconds = DEFAULT_INBOUND_LATENCY;
  rtOpts.outboundLatency.nanoseconds = DEFAULT_OUTBOUND_LATENCY;
  rtOpts.noResetClock = DEFAULT_NO_RESET_CLOCK;
  rtOpts.s = DEFAULT_DELAY_S;
  rtOpts.ap = DEFAULT_AP;
  rtOpts.ai = DEFAULT_AI;
  rtOpts.max_foreign_records = DEFUALT_MAX_FOREIGN_RECORDS;
  rtOpts.currentUtcOffset = DEFAULT_UTC_OFFSET;
  

  if( !(ptpClock = ptpdStartup( &ret, &rtOpts)) )
    return;

  if(rtOpts.probe)
  {
    probe(&rtOpts, ptpClock);
  }
  else
  {
    /* do the protocol engine */
    protocol(&rtOpts, ptpClock);
  }

  ptpdShutdown();

  NOTIFY("self shutdown, probably due to an error\n");
//  return 1;

    while (1)
    {
        /* Put the thread's "main" body HERE */

        /* Use a "break" instruction to exit the "while (1)" loop */
    }

    /* Put the thread's exit from "main" HERE */
    /* A thread is automatically Destroyed when it exits its run function */
}

int
Ptpd_ErrorFunction(void **inPtr)
{

    /* TODO - Put this thread's error handling code HERE */

      /* The default ErrorHandler goes to KernelPanic */

	VDK_CThread_Error(VDK_GetThreadID());
	return 0;
}

void
Ptpd_InitFunction(void **inPtr, VDK_ThreadCreationBlock const *pTCB)
{
    /* Put code to be executed when this thread has just been created HERE */

    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */
}

void
Ptpd_DestroyFunction(void **inPtr)
{
    /* Put code to be executed when this thread is destroyed HERE */

    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */
}

/* ========================================================================== */
