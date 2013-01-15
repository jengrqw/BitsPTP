
//#include <windows.h>
#include "ptpd.h"
#include <time.h>
#include <Iptypes.h>
#include <Iphlpapi.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif


void adjtimex( int adj )
{
	if(adj=0)
		return;

	SetSystemTimeAdjustment(adj, FALSE);



}


int gettimeofday(TimeInternal *tv)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;

//  static int tzflag;
 
  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);
 
    tmpres = ft.dwHighDateTime;
    tmpres <<= 32;
	tmpres |= ft.dwLowDateTime;
 
    /*converting file time to unix epoch*/
    tmpres /= 10;  /*convert into microseconds*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tv->seconds = (long)(tmpres / 1000000UL);
    tv->nanoseconds = (long)(tmpres % 1000000UL)*1000;
   }
 

 
  return 1;
}

int settimeofday(TimeInternal *tv)
{
/*
	SYSTEMTIME st;
	struct tm *gmtm;
	long x = tv->seconds;
	long y = tv->nanoseconds;
  
	gmtm = gmtime((const time_t *) &x);
	st.wSecond		= (WORD) gmtm->tm_sec;
	st.wMinute		= (WORD) gmtm->tm_min;
	st.wHour		= (WORD) gmtm->tm_hour;
	st.wDay			= (WORD) gmtm->tm_mday;
	st.wMonth		= (WORD) (gmtm->tm_mon  + 1);
	st.wYear		= (WORD) (gmtm->tm_year + 1900);
	st.wDayOfWeek		= (WORD) gmtm->tm_wday;
	st.wMilliseconds	= (WORD) (y / 1000000);

	if (!SetSystemTime(&st)) { 
		DBGV("SetSystemTime failed\n");
		return -1;
	}
*/
	FILETIME ft;
	SYSTEMTIME st;
	unsigned __int64 time = 0;

	time = (long long)tv->seconds*1000000UL;
	time += (long long)tv->nanoseconds/1000;
	time +=DELTA_EPOCH_IN_MICROSECS; 
	time *= 10;

	ft.dwLowDateTime = time;
	ft.dwHighDateTime = time>>32;

	if(FileTimeToSystemTime(&ft, &st)==0)
	{
		//failed
		DBGV("FileTimeToSystemTime failed\n");
		return 0;
	}
	
	if (!SetSystemTime(&st)) { 
		DBGV("SetSystemTime failed\n");
		return 0;
	}
	
	return 1;
}


// Fetches the MAC address and prints it
int GetMACaddress(char * mac_addr, int len)
{
	IP_ADAPTER_INFO AdapterInfo[16];       // Allocate information
											// for up to 16 NICs
	PIP_ADAPTER_INFO pAdapterInfo;


	DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(      // Call GetAdapterInfo
									AdapterInfo,      // [out] buffer to receive data
									&dwBufLen);       // [in] size of receive data buffer
	// Verify return value is
	// valid, no buffer overflow
	if(dwStatus != ERROR_SUCCESS)
	{
		return 0;
	}


	pAdapterInfo = AdapterInfo; // Contains pointer to
												// current adapter info

	//copy the first adapter address
	memcpy(mac_addr, pAdapterInfo->Address, len);
	return 1;
}
