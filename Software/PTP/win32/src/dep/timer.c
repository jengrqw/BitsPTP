/* timer.c */

#include "../ptpd.h"

#define TIMER_INTERVAL 1
int elapsed;

void catch_alarm(int sig)
{
  elapsed += TIMER_INTERVAL;
  
 // DBGV("catch_alarm: elapsed %d\n", elapsed);
}

void initTimer(void)
{
	
#if defined (linux)	
  struct itimerval itimer;
  
  
  signal(SIGALRM, SIG_IGN);
  
  elapsed = 0;
  itimer.it_value.tv_sec = itimer.it_interval.tv_sec = TIMER_INTERVAL;
  itimer.it_value.tv_usec = itimer.it_interval.tv_usec = 0;
  
  signal(SIGALRM, catch_alarm);
  setitimer(ITIMER_REAL, &itimer, 0);
  
#else if (WIN32)
  DBG("initTimer\n");
#endif  
}

void timerUpdate(IntervalTimer *itimer)
{
  int i, delta;
  int curTime;
  SYSTEMTIME WinTime;

#ifdef WIN32
 // Integer32 curTime = GetTickCount()*1000;
	GetSystemTime(&WinTime);
	curTime = WinTime.wMinute*60 +WinTime.wSecond;
#else   //linux 
  delta = elapsed;
  elapsed = 0;
#endif


  for(i = 0; i < TIMER_ARRAY_SIZE; ++i)
  {
#ifdef WIN32  	
//    delta = curTime - itimer[i].lastTime;
//    itimer[i].lastTime = curTime;
    delta = curTime - itimer[i].lastTime;
    itimer[i].lastTime = curTime;
#endif
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
  SYSTEMTIME WinTime;
  int curTime;

  if(index >= TIMER_ARRAY_SIZE)
    return;

 
  itimer[index].expire = FALSE;
#if defined linux  
  itimer[index].left = interval;
#else if (WIN32)  
  GetSystemTime(&WinTime);
  curTime = WinTime.wMinute*60 +WinTime.wSecond;
  itimer[index].left = interval;		/*minute-> sec*/
//  itimer[index].left = interval*1000;
#endif  
  itimer[index].interval = itimer[index].left;
 // itimer[index].lastTime = GetTickCount()*1000; /* millisecond */
  itimer[index].lastTime = curTime; /* second */
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

