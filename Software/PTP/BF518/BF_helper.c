// This file includes Blackfin specific funtions
#include <lwip/sockets.h>
#include <cdefbf518.h>
#include "src/dep/constants_dep.h"
#include <ptpd.h>
#include "adi_ssl_init.h"
#include <math.h>


#define PTP_FREQUENCY		50	// PTP clock frequency, MHz
#define GIGA_MAGNITUDE      1000000000LL
#define MEGA_MAGNITUDE      1000000LL


#define PTPEVT_RECEIVE_TIMESTAMP_BIT    0x00000002
#define PTPEVT_TRANSMIT_TIMESTAMP_BIT   0x00000004
#define PTPEVT_RECEIVE_OVERFLOW_BIT   	0x00000010
#define PTPEVT_TRANSMIT_OVERFLOW_BIT   	0x00000020


#define PTPEVT_TRIGGER_DONE_BIT         0x00000008



u64 nanosec_to_pulse(u64 nanosec)
{
	float rate;
	u64 pulse;

	rate = (float) PTP_FREQUENCY/ (float) 1000;
	pulse = (nanosec /1000)*PTP_FREQUENCY;

	return pulse;


}

u64 pulse_to_nanosec(u64 pulse)
{
	int rate;
	u64 nsec;

	rate = 1000/PTP_FREQUENCY;
	nsec = pulse*rate;

	return nsec;


}

////////////////////////////////////////////////////////////////////////
// void Init_BF_PTP_Timer(void)
// Description: Enable PTP_TSYNC module, use SCLK as input clock source
// 				by default. set up addend register value
////////////////////////////////////////////////////////////////////////
void Init_BF_PTP_Timer(void)
{
	u32 Addend_reg_value;
	u32 reg_value;

    double ratio;

    /* Set up addend to generate CLOCK_PTP */
	//caculate ADDEND register value
    ratio = (double)( 1LL<<32 );
    ratio /= ( (double)BF_SYSTEM_CLOCK_MHZ/(double)PTP_FREQUENCY );

    reg_value = ratio;

    *pEMAC_PTP_ADDEND = reg_value;

	//initiate time to 0
	*pEMAC_PTP_TIMELO=0;
	*pEMAC_PTP_TIMEHI=0;

	//read sequenc ID at position 0x48 offset
	*pEMAC_PTP_ID_OFF =0x48;

	////////////////////////////////
	// init blackfin PTP control register
	////////////////////////////////
	/* start the PTP engine */
//	*pEMAC_PTP_CTL |= EMAC_PTP_CTL_EN;  // bit 0

    /* For 1588 V1 on UDP */
    reg_value = 0x4A24170C;     /* Default value of PTP_FOFF */
    *pEMAC_PTP_FOFF = reg_value;

    reg_value = 0x11040800;     /* Default value of PTP_FV1 */
    *pEMAC_PTP_FV1 = reg_value;

    reg_value = 0x0140013f;     /* Default Value of PTP_FV2 */
    *pEMAC_PTP_FV2 = reg_value;

    reg_value = 0xfffffffc;     /* Default Value of PTP_FV3 */
    *pEMAC_PTP_FV3 = reg_value;


    reg_value = 0x0042;     /* Default value of PTP_CTL */
    reg_value |= 1<<7;      /* Enable PPS */

    reg_value &= ~(1<<8);      /* Enable EtherType mask */
    reg_value &= ~(1<<12);     /* Enable Control Field Mask */
    reg_value &= 1<<9;         /* enable IPVM */
    reg_value &= 1<<10;        /* enable IPTM */
    reg_value &= 1<<11;        /* enable UDPEM */

    reg_value |= 1<<13;     /* Enable Clkout */
    reg_value |= 1<<1;      /* Enabe timestamp */
    reg_value |= 1<<0;      /* Enabe the module */
    /*
     * reg_value |= 1<<4;      [> Enabe auxiliary module <]
     */
    *pEMAC_PTP_CTL = reg_value;

    ssync();



}


// The functions gettimeofday and settimeofday can get and set
// the time as well as a timezone.
int gettimeofday(TimeInternal *tv)
{
	//get EMAC_PTP_TIMELO and EMAC_PTP_TIMEHI register in BF518

	u64 clock;
	u64 temp;
	u64 time;
	unsigned long long pulse;

	// read from LO before HI
	clock = *pEMAC_PTP_TIMELO;
	temp = *pEMAC_PTP_TIMEHI;
	temp = temp <<32;
	clock = clock + temp;

	pulse = clock;
    clock = pulse_to_nanosec(clock);

	tv->seconds = clock / GIGA_MAGNITUDE;
	tv->nanoseconds = clock % GIGA_MAGNITUDE;
//	DBG("pulse is %u, clock is %u\n", pulse, clock);
//	DBG("second is %d, nanosec is %d\n", tv->seconds, tv->nanoseconds);

	return 1;
}


int settimeofday(TimeInternal *tv )
{

	u64 temp;
	u64 clock;
	u64 pulse;
	u32 sec;
	u32 nanosec;
	u64 total;
	TimeInternal timeread;


	//set number of total cycles to EMAC_PTP_TIMELO and
	//EMAC_PTP_TIMEHI register in BF518
    temp = tv->seconds;
    clock = tv->nanoseconds + temp * GIGA_MAGNITUDE;

    temp = clock;

    clock = nanosec_to_pulse(clock);

	sec = clock;
	nanosec = clock>>32;

	// write to LO before HI
	*pEMAC_PTP_TIMELO = clock;
	*pEMAC_PTP_TIMEHI = clock >> 32 ;

	//verify the clock
	gettimeofday(&timeread);

//	DBG("Set time:second:%d, nanosec:%d\n",tv->seconds, tv->nanoseconds);
//	DBG("Get time:second:%d, nanosec:%d\n",timeread.seconds, timeread.nanoseconds);

//	DBG("clock:%d, pulse:%d,second:%d, nanosec:%d\n",clock, temp, sec, nanosec);

	return 1;
}

/* The input param adj is nsec/sec, it is to be propotionally converted to the change of Addend dA/A */
bool adjtimex( int adj )
{
    static double temp;
    unsigned int addend;
    long float change_ratio;

    int data;


    addend = 0xA0000000;

    data = pow(2,DEFUALT_SYNC_INTERVAL);

    change_ratio = (long float)adj /(long float)(1e+9);
    change_ratio /= data;

    data = change_ratio*addend;
    addend += data;


    *pEMAC_PTP_ADDEND = addend;

    DBGV( "addend:%08x, adjusted ratio: %.12f, adj: %d\n", addend, change_ratio, adj );


    return true;
}



//The rand() function returns a pseudo-random integer between 0 and RAND_MAX
int rand_r(unsigned int *seedp)
{

	return rand();
}

bool getRxStamp(
    UInteger32 *retNumberOfSeconds,
    UInteger32 *retNumberOfNanoSeconds,
    UInteger16 *overflowCount,
    UInteger16 *sequenceId,
    UInteger16 *messageType,
    UInteger16 *hashValue )
{
    UInteger16 ptp_istat = *pEMAC_PTP_ISTAT;
    UInteger16 sequence_ID;
    if( ptp_istat & PTPEVT_RECEIVE_TIMESTAMP_BIT )
    {
		u64 clock;
		u64 temp;

		// read from LO before HI
		clock = *pEMAC_PTP_RXSNAPLO;
		temp = *pEMAC_PTP_RXSNAPHI;
		temp = temp <<32;
		clock = clock + temp;

    	clock = pulse_to_nanosec(clock);

		*retNumberOfNanoSeconds = clock % GIGA_MAGNITUDE;
		*retNumberOfSeconds 	= clock / GIGA_MAGNITUDE;
		sequence_ID = *pEMAC_PTP_ID_SNAP;
		*sequenceId 			= sequence_ID<<8 & 0xff00;
		*sequenceId 			|= sequence_ID>>8 & 0xff;
        DBGV( "In getRxStamp: overflow=%d, hash=%d, sequenceid=%d, msgType=%d\n", *overflowCount, *hashValue, *sequenceId, *messageType );
        if( ptp_istat & PTPEVT_RECEIVE_OVERFLOW_BIT )
        {
        	*pEMAC_PTP_ISTAT |= PTPEVT_RECEIVE_OVERFLOW_BIT;	//write 1 to clear
            ERROR( "Receive timestamp overflow occured\n" );
        }
        return 1;
    }
    else
        return 0;
}


bool getTxStamp(
    UInteger32 *retNumberOfSeconds,
    UInteger32 *retNumberOfNanoSeconds,
    UInteger16 *overflowCount )
{
    UInteger16 ptp_istat =  *pEMAC_PTP_ISTAT;

    if( ptp_istat & (1<<3) )      /* TXEL bit of status register */
    {
		u64 clock;
		u64 temp;

		// read from LO before HI
		clock = *pEMAC_PTP_TXSNAPLO;
		temp = *pEMAC_PTP_TXSNAPHI;
		temp = temp <<32;
		clock = clock + temp;
		clock = pulse_to_nanosec(clock);


	    *retNumberOfSeconds = clock / GIGA_MAGNITUDE;
	    *retNumberOfNanoSeconds =  clock % GIGA_MAGNITUDE;
	    *overflowCount = ptp_istat & ( 1<<5 );        /* TXOV bit of status register */



        DBGV( "getTxStamp: second=%ld, nanosecond=%ld\n", *retNumberOfSeconds, *retNumberOfNanoSeconds );
        if( *overflowCount != 0 )
        {
            *pEMAC_PTP_ISTAT = 1<<5;        /* clear the overrun bit */
            ERROR( "Transmit timestamp overflow occured\n" );
        }
        return 1;
    }
    else
    {
        return 0;
    }


}

bool checkMsgStamp( unsigned char* buf, UInteger16 sequenceId, UInteger16 messageType )
{

    UInteger16 msgType;
    UInteger16 msgHash;
    UInteger16 msgSequenceId;

    msgSequenceId = buf[31] + buf[30]*256;
    msgType = buf[32];              /* the control field */

    DBGV( "In checkMsgStamp: sequenceid=%d, msgType=%d\n", msgSequenceId, msgType );

    if( msgSequenceId == sequenceId )
        return 1;
    else
        return 0;

}
