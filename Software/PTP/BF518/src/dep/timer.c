/* timer.c */

#include "../ptpd.h"

//#define TIMER_INTERVAL 1
//int elapsed;

void initTimer(void)
{
	
  extern void initLocalClock();
  Init_BF_PTP_Timer();

  DBG("initTimer\n");

}

void timerUpdate(IntervalTimer *itimer)
{
  int i, delta;

    Integer32 curTime = VDK_GetUptime();
    curTime = curTime*VDK_GetTickPeriod();
  
  for(i = 0; i < TIMER_ARRAY_SIZE; ++i)
  {
 	
    delta = curTime - itimer[i].lastTime;
    itimer[i].lastTime = curTime;
  	if(itimer[i].interval > 0 && (itimer[i].left -= delta) <= 0)
    {
      itimer[i].left = itimer[i].interval;
      itimer[i].expire = TRUE;
      DBGV("timerUpdate: timer %u expired\n", i);
    }
  }
}

void timerStop(UInteger16 index, IntervalTimer *itimer)
{
  if(index >= TIMER_ARRAY_SIZE)
    return;
  
  itimer[index].interval = 0;
}

void timerStart(UInteger16 index, UInteger16 interval, IntervalTimer *itimer)
{
  if(index >= TIMER_ARRAY_SIZE)
    return;
  
  itimer[index].expire = FALSE;
  itimer[index].left = interval*1000;		// ticks in VDK is ms

  itimer[index].interval = itimer[index].left;
  itimer[index].lastTime = VDK_GetUptime()*VDK_GetTickPeriod();
  DBGV("timerStart: set timer %d to %d\n", index, interval);
}

Boolean timerExpired(UInteger16 index, IntervalTimer *itimer)
{
  timerUpdate(itimer);
  
  if(index >= TIMER_ARRAY_SIZE)
    return FALSE;
  
  if(!itimer[index].expire)
    return FALSE;
  
  itimer[index].expire = FALSE;
  
  return TRUE;
}

