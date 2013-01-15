/* constants.h */

#ifndef CONSTANTS_H
#define CONSTANTS_H

/* implementation specific constants */
#ifdef __ADSPBF518__
#define MANUFACTURER_ID	\
  "ADSP BF518\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
#else
#define MANUFACTURER_ID \
  "Kendall;1rc1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
#endif

#define DEFUALT_SYNC_INTERVAL        1
#define DEFAULT_UTC_OFFSET           0
#define DEFAULT_CLOCK_VARIANCE       (-4000)
#define DEFAULT_CLOCK_STRATUM        4
#define DEFAULT_INBOUND_LATENCY      215 //0       /* in nsec */
#define DEFAULT_OUTBOUND_LATENCY     0       /* in nsec */
#define DEFAULT_NO_RESET_CLOCK       FALSE
#define DEFAULT_AP                   2  //10
#define DEFAULT_AI                   30 // 1000
#define DEFAULT_DELAY_S              6
#define DEFUALT_MAX_FOREIGN_RECORDS  5

/* features, only change to refelect changes in implementation */
#define CLOCK_FOLLOWUP    TRUE
#define INITIALIZABLE     TRUE
#define BURST_ENABLED     FALSE
#define EXTERNAL_TIMING   FALSE
#define BOUNDARY_CLOCK    FALSE
#define NUMBER_PORTS      1
#define VERSION_PTP       1
#define VERSION_NETWORK   1

/* spec defined constants  */
#define DEFAULT_PTP_DOMAIN_NAME      "_DFLT\0\0\0\0\0\0\0\0\0\0\0"
#define ALTERNATE_PTP_DOMAIN1_NAME   "_ALT1\0\0\0\0\0\0\0\0\0\0\0"
#define ALTERNATE_PTP_DOMAIN2_NAME   "_ALT2\0\0\0\0\0\0\0\0\0\0\0"
#define ALTERNATE_PTP_DOMAIN3_NAME   "_ALT3\0\0\0\0\0\0\0\0\0\0\0"

#define IDENTIFIER_ATOM   "ATOM"
#define IDENTIFIER_GPS    "GPS\0"
#define IDENTIFIER_NTP    "NTP\0"
#define IDENTIFIER_HAND   "HAND"
#define IDENTIFIER_INIT   "INIT"
#define IDENTIFIER_DFLT   "DFLT"

/* ptp constants */
#define PTP_UUID_LENGTH                     6
#define PTP_CODE_STRING_LENGTH              4
#define PTP_SUBDOMAIN_NAME_LENGTH           16
#define PTP_MAX_MANAGEMENT_PAYLOAD_SIZE     90
/* no support for intervals less than one */
#define PTP_SYNC_INTERVAL_TIMEOUT(x)        (1<<((x)<0?1:(x))) 
#define PTP_SYNC_RECEIPT_TIMEOUT(x)         (10*(1<<((x)<0?0:(x))))
#define PTP_DELAY_REQ_INTERVAL              30
#define PTP_FOREIGN_MASTER_THRESHOLD        2
#define PTP_FOREIGN_MASTER_TIME_WINDOW(x)   (4*(1<<((x)<0?0:(x))))
#define PTP_RANDOMIZING_SLOTS               18
#define PTP_LOG_VARIANCE_THRESHOLD          256
#define PTP_LOG_VARIANCE_HYSTERESIS         128
/* used in spec but not named */
#define MANUFACTURER_ID_LENGTH              48

/* ptp data enums */
enum {
  PTP_CLOSED=0,  		/* Closed system outside the scope of this standard */
  PTP_ETHER,  			/* IEEE 802.3™ (Ethernet)*/
  PTP_FFBUS=4,			/* FOUNDATION fieldbus */
  PTP_PROFIBUS,  		/* PROFIBUS */
  PTP_LON,  			/* LonTalk® protocol */
  PTP_DNET,				/* DeviceNet */
  PTP_SDS,  			/* SmartDistributedSystem */
  PTP_CONTROLNET,  		/* ControlNet */
  PTP_CANOPEN,			/* CANopen */
  PTP_IEEE1394=243, 	/* IEEE 1394 */
  PTP_IEEE802_11A,  	/* IEEE 802.11a */
  PTP_IEEE_WIRELESS,	/* IEEE 802.11b */
  PTP_INFINIBAND,  		/* InfiniBand */
  PTP_BLUETOOTH,  		/* Bluetooth™ wireless */
  PTP_IEEE802_15_1,		/* IEEE 802.15.1 */
  PTP_IEEE1451_2,  		/* IEEE 1451.3 */
  PTP_IEEE1451_5,  		/* IEEE 1451.5 */
  PTP_USB,				/* USB bus */
  PTP_ISA,  			/* ISA bus */
  PTP_PCI,  			/* PCI bus */
  PTP_VXI,  			/* VXI bus */
  PTP_DEFAULT			/* Default value*/
};

enum {
  PTP_INITIALIZING=0,  		/*Initialize the data sets, hardware, and communication properties of the clock*/
  PTP_FAULTY,  				/*The fault state of the protocol. No participate in the synchronization*/
  PTP_DISABLED,				/* Do not place any messages on its communication path. */
  PTP_LISTENING,  			/* waiting for the Sync message receipt timeout to expire */
  							/* or to receive a Sync message from a master. */
  PTP_PRE_MASTER,  			/* in the PTP_MASTER state except not place any non-Management messages */
  PTP_MASTER,				/* behaving as a master port */
  PTP_PASSIVE,  			/* shall not place any messages on its communication path */
  PTP_UNCALIBRATED,  		/* a transient state that The appropriate master port is being selected and the */
  							/* local port is preparing to synchronize to the selected master port */
  PTP_SLAVE					/* synchronize to the selected master port */
};

enum {
  PTP_SYNC_MESSAGE=0,  PTP_DELAY_REQ_MESSAGE,  PTP_FOLLOWUP_MESSAGE,
  PTP_DELAY_RESP_MESSAGE,  PTP_MANAGEMENT_MESSAGE,
  PTP_SYNC_MESSAGE_BURST, PTP_DELAY_REQ_MESSAGE_BURST
};

enum {
  PTP_LI_61=0, PTP_LI_59, PTP_BOUNDARY_CLOCK,
  PTP_ASSIST, PTP_EXT_SYNC, PARENT_STATS, PTP_SYNC_BURST
};

enum {
  PTP_MM_NULL=0,  PTP_MM_OBTAIN_IDENTITY,  PTP_MM_CLOCK_IDENTITY,
  PTP_MM_INITIALIZE_CLOCK,  PTP_MM_SET_SUBDOMAIN,
  PTP_MM_CLEAR_DESIGNATED_PREFERRED_MASTER,
  PTP_MM_SET_DESIGNATED_PREFERRED_MASTER,
  PTP_MM_GET_DEFAULT_DATA_SET,  PTP_MM_DEFAULT_DATA_SET,
  PTP_MM_UPDATE_DEFAULT_DATA_SET,  PTP_MM_GET_CURRENT_DATA_SET,
  PTP_MM_CURRENT_DATA_SET,  PTP_MM_GET_PARENT_DATA_SET,
  PTP_MM_PARENT_DATA_SET,  PTP_MM_GET_PORT_DATA_SET,
  PTP_MM_PORT_DATA_SET,  PTP_MM_GET_GLOBAL_TIME_DATA_SET,
  PTP_MM_GLOBAL_TIME_DATA_SET,  PTP_MM_UPDATE_GLOBAL_TIME_PROPERTIES,
  PTP_MM_GOTO_FAULTY_STATE,  PTP_MM_GET_FOREIGN_DATA_SET,
  PTP_MM_FOREIGN_DATA_SET,  PTP_MM_SET_SYNC_INTERVAL,
  PTP_MM_DISABLE_PORT,  PTP_MM_ENABLE_PORT,
  PTP_MM_DISABLE_BURST,  PTP_MM_ENABLE_BURST,  PTP_MM_SET_TIME
};

/* enum used by this implementation */
enum {
  SYNC_RECEIPT_TIMER=0, SYNC_INTERVAL_TIMER, QUALIFICATION_TIMER,
  TIMER_ARRAY_SIZE  /* these two are non-spec */
};

#endif

