/* sys.c */

#include "../ptpd.h"
#include <io.h>

void displayStats(RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
  static int start = 1;
  static char sbuf[SCREEN_BUFSZ];
  char *s;
  int len = 0;
  int size=0;
  
  if(start && rtOpts->csvStats)
  {
    start = 0;
    printf("state, one way delay, offset from master, drift, variance\n");
    fflush(stdout);
  }
  
  memset(sbuf, ' ', SCREEN_BUFSZ);
  
  switch(ptpClock->port_state)
  {
  case PTP_INITIALIZING:  s = "init";  break;
  case PTP_FAULTY:        s = "flt";   break;
  case PTP_LISTENING:     s = "lstn";  break;
  case PTP_PASSIVE:       s = "pass";  break;
  case PTP_UNCALIBRATED:  s = "uncl";  break;
  case PTP_SLAVE:         s = "slv";   break;
  case PTP_PRE_MASTER:    s = "pmst";  break;
  case PTP_MASTER:        s = "mst";   break;
  case PTP_DISABLED:      s = "dsbl";  break;
  default:                s = "?";     break;
  }
  
  size = rtOpts->csvStats ? sizeof("\n"): sizeof("\rstate: \n");
  size += sizeof(s);
  len += sprintf_s(sbuf + len, 
	  size,"%s%s", 
	  rtOpts->csvStats ? "\n": "\rstate: \n", s);
  
  if(ptpClock->port_state == PTP_SLAVE)
  {
	  size = rtOpts->csvStats ? sizeof("") : sizeof("owd: ") + rtOpts->csvStats ? sizeof(""): sizeof("ofm: ");
    len += sprintf_s(sbuf + len,
		size,
      ", %s%d.%09d" ", %s%d.%09d",
      rtOpts->csvStats ? "" : "owd: ",
      ptpClock->one_way_delay.seconds,
      abs(ptpClock->one_way_delay.nanoseconds),
      rtOpts->csvStats ? "" : "ofm: ",
      ptpClock->offset_from_master.seconds,
      abs(ptpClock->offset_from_master.nanoseconds));
    
	size = rtOpts->csvStats ? sizeof("" ): sizeof("drift: ")+rtOpts->csvStats ? sizeof("" ): sizeof("var: ");
    len += sprintf_s(sbuf + len, 
		size,
      ", %s%d" ", %s%d",
      rtOpts->csvStats ? "" : "drift: ", ptpClock->observed_drift,
      rtOpts->csvStats ? "" : "var: ", ptpClock->observed_variance);
  }

  size = rtOpts->csvStats ? len : SCREEN_MAXSZ + 1;
  strncpy_s(sbuf,1, (char*)size, _TRUNCATE);
 // write(1, sbuf, rtOpts->csvStats ? len : SCREEN_MAXSZ + 1);
}
#if 0
Boolean nanoSleep(TimeInternal *t)
{
  struct timespec ts, tr;
  
  ts.tv_sec = t->seconds;
  ts.tv_nsec = t->nanoseconds;
  
  if(nanosleep(&ts, &tr) < 0)
  {
    t->seconds = tr.tv_sec;
    t->nanoseconds = tr.tv_nsec;
    return FALSE;
  }
  
  return TRUE;
}
#endif

void getTime(TimeInternal *time)
{
#if defined (linux)  
  struct timeval tv;
  gettimeofday(&tv, 0);
#else if (WIN32)
//  struct timespec tv;
  gettimeofday(time);
#endif  

}

void setTime(TimeInternal *time)
{
  struct timeval tv;
  
  tv.tv_sec = time->seconds;
  tv.tv_usec = time->nanoseconds;
  settimeofday(time);
  
  NOTIFY("resetting system clock to %ds %dns\n", time->seconds, time->nanoseconds);
}

UInteger16 getRand(UInteger32 *seed)
{
  return rand();
}

Boolean adjFreq(Integer32 adj)
{
	
#ifdef linux	
  struct timex t;
  
  if(adj > ADJ_FREQ_MAX)
    adj = ADJ_FREQ_MAX;
  else if(adj < -ADJ_FREQ_MAX)
    adj = -ADJ_FREQ_MAX;
  
  t.modes = MOD_FREQUENCY;
  t.freq = adj*((1<<16)/1000);
  
  
  /*The Linux system call adjtimex reads and optionally sets adjustment 
  parameters for this algorithm. */
  return !adjtimex(&t);
#elif WIN32
	SetSystemTimeAdjustment(adj, TRUE);
  DBGV("adj:      %10is\n", adj);
	return 1;


  
#endif  
}

