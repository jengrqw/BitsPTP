/* ptpd_dep.h */

#ifndef PTPD_DEP_H
#define PTPD_DEP_H

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<time.h>

#include <stddef.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>


/* system messages */
#define ERROR(x, ...)  fprintf(stderr, "(ptpd error) " x, ##__VA_ARGS__)
#define PERROR(x, ...) fprintf(stderr, "(ptpd error) " x ": %m\n", ##__VA_ARGS__)
#define NOTIFY(x, ...) fprintf(stderr, "(ptpd notice) " x, ##__VA_ARGS__)

/* debug messages */
#ifdef PTPD_DBGV
#define PTPD_DBG
#ifdef __ADSPBF518__
//#define DBGV(x, ...)  printf("(ptpd debug) " x, ##__VA_ARGS__)
#define DBGV(x, ...)  U_DEBUG_PRINT("(ptpd debug) " x, ##__VA_ARGS__)

#else
#define DBGV(x, ...) fprintf(stderr, "(ptpd debug) " x, ##__VA_ARGS__)
#endif
#else
#define DBGV(x, ...)
#endif

#ifdef PTPD_DBG
#ifdef __ADSPBF518__
//#define DBG(x, ...)  printf("(ptpd debug) " x, ##__VA_ARGS__)
#define DBG(x, ...)  U_DEBUG_PRINT("(ptpd debug) " x, ##__VA_ARGS__)
#else
#define DBG(x, ...)  fprintf(stderr, "(ptpd debug) " x, ##__VA_ARGS__)
#endif
#else
#define DBG(x, ...)
#endif

/* endian corrections */
#if defined(PTPD_MSBF)
#define shift8(x,y)   ( (x) << ((3-y)<<3) )
#define shift16(x,y)  ( (x) << ((1-y)<<4) )
#elif defined(PTPD_LSBF)
#define shift8(x,y)   ( (x) << ((y)<<3) )
#define shift16(x,y)  ( (x) << ((y)<<4) )
#endif

#define flip16(x) htons(x)
#define flip32(x) htonl(x)


/* bit array manipulation */
#define getFlag(x,y)  !!( *(UInteger8*)((x)+((y)<8?1:0)) &   (1<<((y)<8?(y):(y)-8)) )
#define setFlag(x,y)    ( *(UInteger8*)((x)+((y)<8?1:0)) |=   1<<((y)<8?(y):(y)-8)  )
#define clearFlag(x,y)  ( *(UInteger8*)((x)+((y)<8?1:0)) &= ~(1<<((y)<8?(y):(y)-8)) )


/* msg.c */
Boolean msgPeek(void*,ssize_t);
void msgUnpackHeader(void*,MsgHeader*);
void msgUnpackSync(void*,MsgSync*);
void msgUnpackDelayReq(void*,MsgDelayReq*);
void msgUnpackFollowUp(void*,MsgFollowUp*);
void msgUnpackDelayResp(void*,MsgDelayResp*);
void msgUnpackManagement(void*,MsgManagement*);
UInteger8 msgUnloadManagement(void*,MsgManagement*,PtpClock*,RunTimeOpts*);
void msgUnpackManagementPayload(void *buf, MsgManagement *manage);
void msgPackHeader(void*,PtpClock*);
void msgPackSync(void*,Boolean,TimeRepresentation*,PtpClock*);
void msgPackDelayReq(void*,Boolean,TimeRepresentation*,PtpClock*);
void msgPackFollowUp(void*,UInteger16,TimeRepresentation*,PtpClock*);
void msgPackDelayResp(void*,MsgHeader*,TimeRepresentation*,PtpClock*);
UInteger16 msgPackManagement(void*,MsgManagement*,PtpClock*);
UInteger16 msgPackManagementResponse(void*,MsgHeader*,MsgManagement*,PtpClock*);

/* net.c */
/* linux API dependent */
Boolean netInit(NetPath*,RunTimeOpts*,PtpClock*);
Boolean netShutdown(NetPath*);
int netSelect(TimeInternal*,NetPath*);
ssize_t netRecvEvent(Octet*,TimeInternal*,NetPath*);
ssize_t netRecvGeneral(Octet*,NetPath*);
ssize_t netSendEvent(Octet*,UInteger16,NetPath*);
ssize_t netSendGeneral(Octet*,UInteger16,NetPath*);

/* servo.c */
void initClock(RunTimeOpts*,PtpClock*);
void updateDelay(TimeInternal*,TimeInternal*,
  one_way_delay_filter*,RunTimeOpts*,PtpClock*);
void updateOffset(TimeInternal*,TimeInternal*,
  offset_from_master_filter*,RunTimeOpts*,PtpClock*);
void updateClock(RunTimeOpts*,PtpClock*);

/* startup.c */
/* unix API dependent */
//PtpClock * ptpdStartup(int,char**,Integer16*,RunTimeOpts*);
PtpClock * ptpdStartup(Integer16*,RunTimeOpts*);
void ptpdShutdown(void);

/* sys.c */
/* unix API dependent */
void displayStats(RunTimeOpts*,PtpClock*);
Boolean nanoSleep(TimeInternal*);
void getTime(TimeInternal*);
void setTime(TimeInternal*);
UInteger16 getRand(UInteger32*);
Boolean adjFreq(Integer32);

/* timer.c */
void initTimer(void);
void timerUpdate(IntervalTimer*);
void timerStop(UInteger16,IntervalTimer*);
void timerStart(UInteger16,UInteger16,IntervalTimer*);
Boolean timerExpired(UInteger16,IntervalTimer*);


/* arith.c */
UInteger32 crc_algorithm(Octet*,Integer16);
UInteger32 sum(Octet*,Integer16);
void fromInternalTime(TimeInternal*,TimeRepresentation*,Boolean);
void toInternalTime(TimeInternal*,TimeRepresentation*,Boolean*);
void normalizeTime(TimeInternal*);
void addTime(TimeInternal*,TimeInternal*,TimeInternal*);
void subTime(TimeInternal*,TimeInternal*,TimeInternal*);

/* bmc.c */
UInteger8 bmc(ForeignMasterRecord*,RunTimeOpts*,PtpClock*);
void m1(PtpClock*);
void s1(MsgHeader*,MsgSync*,PtpClock*);
void initData(RunTimeOpts*,PtpClock*);

/* probe.c */
void probe(RunTimeOpts*,PtpClock*);

/* protocol.c */
void protocol(RunTimeOpts*,PtpClock*);


/* BF518 specific functions */
void Init_BF_PTP_Timer(void);
bool getRxStamp(
    UInteger32 *retNumberOfSeconds,
    UInteger32 *retNumberOfNanoSeconds,
    UInteger16 *overflowCount,
    UInteger16 *sequenceId,
    UInteger16 *messageType,
    UInteger16 *hashValue );
bool checkMsgStamp( unsigned char* buf, UInteger16 sequenceId, UInteger16 messageType ); 

bool getTxStamp(
    UInteger32 *retNumberOfSeconds,
    UInteger32 *retNumberOfNanoSeconds,
    UInteger16 *overflowCount );
int gettimeofday(TimeInternal *tv);
int settimeofday(TimeInternal *tv );

bool adjtimex( int adj );
int rand_r(unsigned int *seedp);
#endif

