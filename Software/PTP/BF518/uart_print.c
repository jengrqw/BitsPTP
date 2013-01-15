/*******************************************************************
Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software
you agree to the terms of the associated Analog Devices License Agreement.

Project Name:  	Power_On_Self_Test

Hardware:		ADSP-BF518F EZ-Board

Description:	This file tests the UART0 interface on the EZ-Board.
*******************************************************************/

/*******************************************************************
*  include files
*******************************************************************/
#include <cdefBF518.h>
#include <ccblkfn.h>
//#include "timer_isr.h"
#include "debug.h"
#include <vdk.h>

/*******************************************************************
*  global variables and defines
*******************************************************************/
/* Apply formula DL = PERIOD / (16 x 8 bits) and call uart_init that writes
	the result to the two 8-bit DL registers (DLH:DLL). */
#define BAUD_RATE_115200 	(0x15A2 >> 7)
#define MAX_TEST_CHARS		5000

/* if we are using the UART to print debug info, define the following */
#ifdef __DEBUG_UART__
//#define UART_DEBUG_BUFFER_LINE_SIZE 512
char UART_DEBUG_BUFFER[UART_DEBUG_BUFFER_LINE_SIZE];
#endif

#if defined(__DEBUG_FILE__)
FILE *pDebugFile;				/* debug file */
#endif

/*******************************************************************
*  function prototypes
*******************************************************************/
void Init_UART(void);
int PutChar(const char c);
int GetChar(char *const c);
int TEST_UART(void);

/* if we are using the UART to print debug info, define the following */
#ifdef __DEBUG_UART__
int UART_DEBUG_PRINT(void);
#endif


/*******************************************************************
*   Function:    Init_UART
*   Description: Initialize UART with the appropriate values
*******************************************************************/

void Init_UART(void)
{
	volatile int temp;

	*pPORTG_FER |= 0x0600;
    ssync();

	/* configure UART0 RX and UART0 TX pins */
	*pPORTG_MUX |= 0x400;
    ssync();

/*****************************************************************************
 *
 *  First of all, enable UART clock.
 *
 ****************************************************************************/
	*pUART0_GCTL = UCEN;

/*****************************************************************************
 *
 *  Read period value and apply formula:  DL = PERIOD / 16 / 8
 *  Write result to the two 8-bit DL registers (DLH:DLL).
 *
 ****************************************************************************/
	*pUART0_LCR = DLAB;
	*pUART0_DLL = BAUD_RATE_115200;
	*pUART0_DLH = (BAUD_RATE_115200 >> 8);

/*****************************************************************************
 *
 *  Clear DLAB again and set UART frame to 8 bits, no parity, 1 stop bit.
 *  This may differ in other scenarios.
 *
 ****************************************************************************/
	*pUART0_LCR = 0x03;

/*****************************************************************************
 *
 *  Finally enable interrupts inside UART module, by setting proper bits
 *  in the IER register. It is good programming style to clear potential
 *  UART interrupt latches in advance, by reading RBR, LSR and IIR.
 *
 *  Setting the ETBEI bit automatically fires a TX interrupt request.
 *
 ****************************************************************************/
	temp = *pUART0_RBR;
	temp = *pUART0_LSR;

	*pUART0_IER = ETBEI ;
}



/*******************************************************************
*   Function:    PutChar
*   Description: Writes a character to the UART.
*******************************************************************/

int PutChar(const char cVal)
{
	int nStatus = 0;
	unsigned int nTimer = VDK_GetUptime();
	int i=0;

	do{
		i++;
		if( (*pUART0_LSR & THRE) )
		{
			*pUART0_THR = cVal;
			nStatus = 1;
			break;
		}
	}while( VDK_GetUptime()<nTimer+100000 );


	return nStatus;
}


#ifdef __DEBUG_UART__
/*******************************************************************
*   Function:    UART_DEBUG_PRINT
*   Description: Prints debug info over the UART using a predefined
*				 buffer.
*******************************************************************/
int UART_DEBUG_PRINT(void)
{
	unsigned int i = 0;		/* index */
	char temp;				/* temp char */


/*	return PutString(UART_DEBUG_BUFFER, UART_DEBUG_BUFFER_LINE_SIZE); */

	/* loop through the debug buffer until the end, a NULL, or an error */
	for ( i = 0; i < UART_DEBUG_BUFFER_LINE_SIZE; i++)
	{
		temp = UART_DEBUG_BUFFER[i];

		/* if not NULL then print it */
		if (temp)
		{
			if( 0 == PutChar(temp) )
			{
				/* if error was detected then quit */
				return 0;
			}

			/* if it was a newline we need to add a carriage return */
			if ( 0x0a == temp )
			{
				if( 0 == PutChar(0x0d) )
				{
					/* if error was detected then quit */
					return 0;
				}
			}
		}
		else
		{
			/* else NULL was found */
			return 1;
		}
	}

	return 1;
}
#endif
