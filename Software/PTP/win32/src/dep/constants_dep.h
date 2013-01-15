/* constants_dep.h */

#ifndef CONSTANTS_DEP_H
#define CONSTANTS_DEP_H

/* platform dependent */

//#if !defined(linux) && !defined(__NetBSD__) && !defined(__FreeBSD__)
#if !defined WIN32
#error Not ported to this architecture, please update.
#endif

#ifdef	linux
#include<netinet/in.h>
#include<net/if.h>
#include<net/if_arp.h>
#define IFACE_NAME_LENGTH         IF_NAMESIZE
#define NET_ADDRESS_LENGTH        INET_ADDRSTRLEN

#define IFCONF_LENGTH 10

#include<endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define PTPD_LSBF
#elif __BYTE_ORDER == __BIG_ENDIAN
#define PTPD_MSBF
#endif
#endif /* linux */


/*********************/
#ifdef WIN32


//#include <stddef.h>
//#include <services_types.h>

#define PTPD_LSBF

#define IFACE_NAME			"Windows"	
#define IFACE_NAME_LENGTH 	16
#define NET_ADDRESS_LENGTH	16

#define ssize_t		size_t
typedef char* 		caddr_t;

#define EINTR            4      /* Interrupted system call */

#define MSG_CTRUNC      8
#define MSG_TRUNC       0x20


//#define ADJ_FREQUENCY           0x0002  /* frequency offset */
//#define MOD_FREQUENCY   ADJ_FREQUENCY

#define ITIMER_REAL             0
#define ITIMER_VIRTUAL          1
#define ITIMER_PROF             2

#define CMSG_NXTHDR(mhdr, cmsg) cmsg_nxthdr((mhdr), (cmsg))
#define CMSG_ALIGN(len) ( ((len)+sizeof(long)-1) & ~(sizeof(long)-1) )
#define CMSG_DATA_WIN(cmsg) ((void *)((char *)(cmsg) + CMSG_ALIGN(sizeof(struct cmsghdr))))
#define __CMSG_FIRSTHDR(ctl,len) ((len) >= sizeof(struct cmsghdr) ? \
                                  (struct cmsghdr *)(ctl) : \
                                  (struct cmsghdr *)NULL)
#define CMSG_FIRSTHDR(msg)      __CMSG_FIRSTHDR((msg)->msg_control, (msg)->msg_controllen)


#endif // WIN32

#if defined(__NetBSD__) || defined(__FreeBSD__)
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <net/if.h>
# include <net/if_dl.h>
# include <net/if_types.h>
# if defined(__FreeBSD__)
#  include <net/ethernet.h>
#  include <sys/uio.h>
# else
#  include <net/if_ether.h>
# endif
# include <ifaddrs.h>
# define IFACE_NAME_LENGTH         IF_NAMESIZE
# define NET_ADDRESS_LENGTH        INET_ADDRSTRLEN

# define IFCONF_LENGTH 10

# define adjtimex ntp_adjtime

# include <machine/endian.h>
# if BYTE_ORDER == LITTLE_ENDIAN
#   define PTPD_LSBF
# elif BYTE_ORDER == BIG_ENDIAN
#   define PTPD_MSBF
# endif
#endif


#define ADJ_FREQ_MAX  512000

/* UDP/IPv4 dependent */

#define SUBDOMAIN_ADDRESS_LENGTH  4
#define PORT_ADDRESS_LENGTH       2

#define PACKET_SIZE  300

#define PTP_EVENT_PORT    319
#define PTP_GENERAL_PORT  320
//#define PTP_EVENT_PORT    1319
//#define PTP_GENERAL_PORT  1320

#define DEFAULT_PTP_DOMAIN_ADDRESS     "224.0.1.129"
#define ALTERNATE_PTP_DOMAIN1_ADDRESS  "224.0.1.130"
#define ALTERNATE_PTP_DOMAIN2_ADDRESS  "224.0.1.131"
#define ALTERNATE_PTP_DOMAIN3_ADDRESS  "224.0.1.132"

#define HEADER_LENGTH             40
#define SYNC_PACKET_LENGTH        124
#define DELAY_REQ_PACKET_LENGTH   124
#define FOLLOW_UP_PACKET_LENGTH   52
#define DELAY_RESP_PACKET_LENGTH  60
#define MANAGEMENT_PACKET_LENGTH  136

#define MM_STARTING_BOUNDARY_HOPS  0x7fff

/* others */

#define SCREEN_BUFSZ  128
#define SCREEN_MAXSZ  80


#endif

