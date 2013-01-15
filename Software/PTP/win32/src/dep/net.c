/* net.c */

#include "../ptpd.h"
#include <Ws2tcpip.h>
#include <Iptypes.h>
#include <Iphlpapi.h>

Boolean lookupSubdomainAddress(Octet *subdomainName, Octet *subdomainAddress)
{
  UInteger32 h;
  
  /* set multicast group address based on subdomainName */
  if (!memcmp(subdomainName, DEFAULT_PTP_DOMAIN_NAME, PTP_SUBDOMAIN_NAME_LENGTH))
    memcpy(subdomainAddress, DEFAULT_PTP_DOMAIN_ADDRESS, NET_ADDRESS_LENGTH);
  else if(!memcmp(subdomainName, ALTERNATE_PTP_DOMAIN1_NAME, PTP_SUBDOMAIN_NAME_LENGTH))
    memcpy(subdomainAddress, ALTERNATE_PTP_DOMAIN1_ADDRESS, NET_ADDRESS_LENGTH);
  else if(!memcmp(subdomainName, ALTERNATE_PTP_DOMAIN2_NAME, PTP_SUBDOMAIN_NAME_LENGTH))
    memcpy(subdomainAddress, ALTERNATE_PTP_DOMAIN2_ADDRESS, NET_ADDRESS_LENGTH);
  else if(!memcmp(subdomainName, ALTERNATE_PTP_DOMAIN3_NAME, PTP_SUBDOMAIN_NAME_LENGTH))
    memcpy(subdomainAddress, ALTERNATE_PTP_DOMAIN3_ADDRESS, NET_ADDRESS_LENGTH);
  else
  {
    h = crc_algorithm(subdomainName, PTP_SUBDOMAIN_NAME_LENGTH) % 3;
    switch(h)
    {
    case 0:
      memcpy(subdomainAddress, ALTERNATE_PTP_DOMAIN1_ADDRESS, NET_ADDRESS_LENGTH);
      break;
    case 1:
      memcpy(subdomainAddress, ALTERNATE_PTP_DOMAIN2_ADDRESS, NET_ADDRESS_LENGTH);
      break;
    case 2:
      memcpy(subdomainAddress, ALTERNATE_PTP_DOMAIN3_ADDRESS, NET_ADDRESS_LENGTH);
      break;
    default:
      ERROR("handle out of range for '%s'!\n", subdomainName);
      return FALSE;
    }
  }
  
  return TRUE;
}

UInteger8 lookupCommunicationTechnology(UInteger8 communicationTechnology)
{
#if defined(linux)
  
  switch(communicationTechnology)
  {
  case ARPHRD_ETHER:
  case ARPHRD_EETHER:
  case ARPHRD_IEEE802:
    return PTP_ETHER;
    
  default:
    break;
  }
  
#elif defined(BSD_INTERFACE_FUNCTIONS)
  
#endif
  
  return PTP_DEFAULT;
}

UInteger32 findIface(Octet *ifaceName, UInteger8 *communicationTechnology,
  Octet *uuid, NetPath *netPath)
{
#if defined(linux)

  /* depends on linux specific ioctls (see 'netdevice' man page) */
  int i, flags;
  struct ifconf data;
  struct ifreq device[IFCONF_LENGTH];
  
  data.ifc_len = sizeof(device);
  data.ifc_req = device;
  
  memset(data.ifc_buf,0,data.ifc_len);
  
  flags = IFF_UP|IFF_RUNNING|IFF_MULTICAST;
  
  /* look for an interface if none specified */
  if(ifaceName[0] != '\0')
  {
    i = 0;
    memcpy(device[i].ifr_name, ifaceName, IFACE_NAME_LENGTH);
    
    if(ioctl(netPath->eventSock, SIOCGIFHWADDR, &device[i]) < 0)
      DBGV("failed to get hardware address\n");
    else if((*communicationTechnology = lookupCommunicationTechnology(device[i].ifr_hwaddr.sa_family)) == PTP_DEFAULT)
      DBGV("unsupported communication technology (%d)\n", *communicationTechnology);
    else
      memcpy(uuid, device[i].ifr_hwaddr.sa_data, PTP_UUID_LENGTH);
  }
  else
  {
    /* no iface specified */
    /* get list of network interfaces*/
    if(ioctl(netPath->eventSock, SIOCGIFCONF, &data) < 0)
    {
      PERROR("failed query network interfaces\n");
      return 0;
    }
    
    if(data.ifc_len >= sizeof(device))
      DBG("device list may exceed allocated space\n");
    
    /* search through interfaces */
    for(i=0; i < data.ifc_len/sizeof(device[0]); ++i)
    {
      DBGV("%d %s %s\n",i,device[i].ifr_name,inet_ntoa(((struct sockaddr_in *)&device[i].ifr_addr)->sin_addr));
      
      if(ioctl(netPath->eventSock, SIOCGIFFLAGS, &device[i]) < 0)
        DBGV("failed to get device flags\n");
      else if((device[i].ifr_flags&flags) != flags)
        DBGV("does not meet requirements (%08x, %08x)\n", device[i].ifr_flags, flags);
      else if(ioctl(netPath->eventSock, SIOCGIFHWADDR, &device[i]) < 0)
        DBGV("failed to get hardware address\n");
      else if((*communicationTechnology = lookupCommunicationTechnology(device[i].ifr_hwaddr.sa_family)) == PTP_DEFAULT)
        DBGV("unsupported communication technology (%d)\n", *communicationTechnology);
      else
      {
        DBGV("found interface (%s)\n", device[i].ifr_name);
        
        memcpy(uuid, device[i].ifr_hwaddr.sa_data, PTP_UUID_LENGTH);
        memcpy(ifaceName, device[i].ifr_name, IFACE_NAME_LENGTH);
        
        break;
      }
    }
  }
  
  if(ifaceName[0] == '\0')
  {
    ERROR("failed to find a usable interface\n");
    return 0;
  }
  
  if(ioctl(netPath->eventSock, SIOCGIFADDR, &device[i]) < 0)
  {
    PERROR("failed to get ip address\n");
    return 0;
  }
  
  return ((struct sockaddr_in *)&device[i].ifr_addr)->sin_addr.s_addr;

#elif defined(BSD_INTERFACE_FUNCTIONS)

  struct ifaddrs *if_list, *ifv4, *ifh;

  if (getifaddrs(&if_list) < 0)
  {
    PERROR("getifaddrs() failed\n");
    return FALSE;
  }

  /* find an IPv4, multicast, UP interface, right name(if supplied) */
  for (ifv4 = if_list; ifv4 != NULL; ifv4 = ifv4->ifa_next)
  {
    if ((ifv4->ifa_flags & IFF_UP) == 0)
      continue;
    if ((ifv4->ifa_flags & IFF_RUNNING) == 0)
      continue;
    if ((ifv4->ifa_flags & IFF_LOOPBACK))
      continue;
    if ((ifv4->ifa_flags & IFF_MULTICAST) == 0)
      continue;
    if (ifv4->ifa_addr->sa_family != AF_INET)  /* must have IPv4 address */
      continue;

    if (ifaceName[0] && strncmp(ifv4->ifa_name, ifaceName, IF_NAMESIZE) != 0)
      continue;

    break;
  }

  if (ifv4 == NULL)
  {
    if (ifaceName[0])
    {
      ERROR("interface \"%s\" does not exist, or is not appropriate\n", ifaceName);
      return FALSE;
    }
    ERROR("no suitable interfaces found!\n");
    return FALSE;
  }

  /* find the AF_LINK info associated with the chosen interface */
  for (ifh = if_list; ifh != NULL; ifh = ifh->ifa_next)
  {
    if (ifh->ifa_addr->sa_family != AF_LINK)
      continue;
    if (strncmp(ifv4->ifa_name, ifh->ifa_name, IF_NAMESIZE) == 0)
      break;
  }

  if (ifh == NULL)
  {
    ERROR("could not get hardware address for interface \"%s\"\n", ifv4->ifa_name);
    return FALSE;
  }

  /* check that the interface TYPE is OK */
  if ( ((struct sockaddr_dl *)ifh->ifa_addr)->sdl_type != IFT_ETHER )
  {
    ERROR("\"%s\" is not an ethernet interface!\n", ifh->ifa_name);
    return FALSE;
  }

  printf("==> %s %s %s\n", ifv4->ifa_name,
       inet_ntoa(((struct sockaddr_in *)ifv4->ifa_addr)->sin_addr),
        ether_ntoa((struct ether_addr *)LLADDR((struct sockaddr_dl *)ifh->ifa_addr))
        );

  *communicationTechnology = PTP_ETHER;
  memcpy(ifaceName, ifh->ifa_name, IFACE_NAME_LENGTH);
  memcpy(uuid, LLADDR((struct sockaddr_dl *)ifh->ifa_addr), PTP_UUID_LENGTH);

  return ((struct sockaddr_in *)ifv4->ifa_addr)->sin_addr.s_addr;
#else if (WIN32)
/*	
	GetMACaddress(uuid, PTP_UUID_LENGTH);
 //   memcpy( uuid, hwaddr, PTP_UUID_LENGTH );     
    *communicationTechnology = PTP_ETHER;

    memcpy(ifaceName, IFACE_NAME, IFACE_NAME_LENGTH);       
*/
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
	memcpy(uuid, pAdapterInfo->Address, PTP_UUID_LENGTH);

	memcpy(ifaceName, pAdapterInfo->Description, IFACE_NAME_LENGTH); 
	*communicationTechnology = PTP_ETHER;

    return TRUE;        /* Ture successful return */


#endif
}

/* start all of the UDP stuff */
/* must specify 'subdomainName', optionally 'ifaceName', if not then pass ifaceName == "" */
/* returns other args */
/* on socket options, see the 'socket(7)' and 'ip' man pages */
Boolean netInit(NetPath *netPath, RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
  int temp, i;
  struct in_addr interfaceAddr, netAddr;
  struct sockaddr_in addr;
  struct ip_mreq imr;
  char addrStr[NET_ADDRESS_LENGTH];
  char *s;
  WSADATA wsaData;
  int NonBlock;

  
  DBG("netInit\n");
  //-----------------------------------------------
  // Initialize Winsock
  WSAStartup(MAKEWORD(2,2), &wsaData);

  
  /* open sockets */
  if( (netPath->eventSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP) ) < 0
    || (netPath->generalSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP) ) < 0 )
  {
    PERROR("failed to initalize sockets\n");
    return FALSE;
  }

  /* find a network interface */
  if( !(interfaceAddr.s_addr = findIface(rtOpts->ifaceName, &ptpClock->port_communication_technology,
    ptpClock->port_uuid_field, netPath)) )
    return FALSE;
  

  /* bind sockets */
  /* need INADDR_ANY to allow receipt of multi-cast and uni-cast messages */
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(PTP_EVENT_PORT);
  if(bind(netPath->eventSock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0)
  {
    PERROR("failed to bind event socket\n");
    return FALSE;
  }
  
  addr.sin_port = htons(PTP_GENERAL_PORT);
  if(bind(netPath->generalSock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0)
  {
    PERROR("failed to bind general socket\n");
    return FALSE;
  }

  /* set general and port address */
  *(Integer16*)ptpClock->event_port_address = PTP_EVENT_PORT;
  *(Integer16*)ptpClock->general_port_address = PTP_GENERAL_PORT;
  
  /* send a uni-cast address if specified (useful for testing) */
  if(rtOpts->unicastAddress[0])
  {
#ifdef linux  	
    if(!inet_aton(rtOpts->unicastAddress, &netAddr))
#elif WIN32
    DBG( "Unicast address: %s\n", rtOpts->unicastAddress );
    if( ( netAddr.s_addr = inet_addr( rtOpts->unicastAddress ) ) == -1  )  
#endif
    {
      ERROR("failed to encode uni-cast address: %s\n", rtOpts->unicastAddress);
      return FALSE;
    }
    
    netPath->unicastAddr = netAddr.s_addr;
  }
  else
    netPath->unicastAddr = 0;
  
  /* resolve PTP subdomain */
  if(!lookupSubdomainAddress(rtOpts->subdomainName, addrStr))
    return FALSE;
#ifdef linux    
  if(!inet_aton(addrStr, &netAddr))
#elif   WIN32
  if( ( netAddr.s_addr = inet_addr( addrStr) ) == -1 )
#endif
  {
    ERROR("failed to encode multi-cast address: %s\n", addrStr);
    return FALSE;
  }
  
  netPath->multicastAddr = netAddr.s_addr;
  
  s = addrStr;
  for(i = 0; i < SUBDOMAIN_ADDRESS_LENGTH; ++i)
  {
    ptpClock->subdomain_address[i] = (char)strtol(s, &s, 0);
    
    if(!s)
      break;
    
    ++s;
  }

  /* set socket to non blocking */
	NonBlock = 1;
	if (ioctlsocket(netPath->eventSock, FIONBIO, &NonBlock) == SOCKET_ERROR||
	ioctlsocket(netPath->generalSock, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		PERROR("ioctlsocket() failed \n");
		return FALSE;
	}


    

  /* multicast send only on specified interface */
 // imr.imr_multiaddr.s_addr = netAddr.s_addr;
 // imr.imr_interface.s_addr = interfaceAddr.s_addr;
 /*   
  if( setsockopt(netPath->eventSock, IPPROTO_IP, IP_MULTICAST_IF, (char*)&imr.imr_interface.s_addr, sizeof(struct in_addr)) < 0
    || setsockopt(netPath->generalSock, IPPROTO_IP, IP_MULTICAST_IF, (char*)&imr.imr_interface.s_addr, sizeof(struct in_addr)) < 0 )
  {
    PERROR("failed to enable multi-cast on the interface\n");
    return FALSE;
  }
  */
  /* multicast send only on specified interface */
  imr.imr_multiaddr.s_addr = netAddr.s_addr;
  imr.imr_interface.s_addr = htonl(INADDR_ANY);

  /* join multicast group (for receiving) on specified interface */
  if( setsockopt(netPath->eventSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&imr, sizeof(struct ip_mreq))  < 0
    || setsockopt(netPath->generalSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&imr, sizeof(struct ip_mreq)) < 0 )
  {
    PERROR("failed to join the multi-cast group, error code %d\n", WSAGetLastError());
    return FALSE;
  }

    /* enable loopback */
  temp = 1;
  
  if( setsockopt(netPath->eventSock, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&temp, sizeof(int)) < 0
    || setsockopt(netPath->generalSock, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&temp, sizeof(int)) < 0 )
  {
    PERROR("failed to enable multi-cast loopback\n");
    return FALSE;
  }


#if 1 
  temp =1;
  if( setsockopt(netPath->eventSock, SOL_SOCKET, SO_REUSEADDR, (char*)&temp, sizeof(int)) < 0
    || setsockopt(netPath->generalSock, SOL_SOCKET, SO_REUSEADDR, (char*)&temp, sizeof(int)) < 0 )
  {
    PERROR("failed to enable multi-cast on the interface\n");
    return FALSE;
  }
  

  /* set socket time-to-live to 1 */
  temp = 1;
  if( setsockopt(netPath->eventSock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&temp, sizeof(int)) < 0
    || setsockopt(netPath->generalSock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&temp, sizeof(int)) < 0 )
  {
    PERROR("failed to set the multi-cast time-to-live\n");
    return FALSE;
  }
  
  /* enable loopback */
  temp = 1;
  
  if( setsockopt(netPath->eventSock, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&temp, sizeof(int)) < 0
    || setsockopt(netPath->generalSock, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&temp, sizeof(int)) < 0 )
  {
    PERROR("failed to enable multi-cast loopback\n");
    return FALSE;
  }

  /* make timestamps available through recvmsg() */
  temp = 1;
  /*
  if( setsockopt(netPath->eventSock, SOL_SOCKET, SO_TIMESTAMP, (char*)&temp, sizeof(int)) < 0
    || setsockopt(netPath->generalSock, SOL_SOCKET, SO_TIMESTAMP, (char*)&temp, sizeof(int)) < 0 )
  {
    PERROR("failed to enable receive time stamps, error code is %d\n",WSAGetLastError());
    return FALSE;
  }

*/
#endif
  return TRUE;
}

/* shut down the UDP stuff */
Boolean netShutdown(NetPath *netPath)
{
  struct ip_mreq imr;

  imr.imr_multiaddr.s_addr = netPath->multicastAddr;
  imr.imr_interface.s_addr = htonl(INADDR_ANY);

  setsockopt(netPath->eventSock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&imr, sizeof(struct ip_mreq));
  setsockopt(netPath->generalSock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&imr, sizeof(struct ip_mreq));
  
  netPath->multicastAddr = 0;
  netPath->unicastAddr = 0;
  
  if(netPath->eventSock > 0)
  {
    closesocket(netPath->eventSock);
    WSACleanup(); 
  }
  netPath->eventSock = -1;
  
  if(netPath->generalSock > 0)
  {
    closesocket(netPath->generalSock);
	WSACleanup(); 
  }
  netPath->generalSock = -1;
    
  return TRUE;
}

int netSelect(TimeInternal *timeout, NetPath *netPath)
{
  int ret; 
  SOCKET nfds;
  struct fd_set readfds;
  struct timeval tv, *tv_ptr;

  if(timeout < 0)
    return FALSE;

  FD_ZERO(&readfds);
  FD_SET(netPath->eventSock, &readfds);
  FD_SET(netPath->generalSock, &readfds);

  if(timeout)
  {
    tv.tv_sec = timeout->seconds;
    tv.tv_usec = timeout->nanoseconds/1000;
    tv_ptr = &tv;
  }
  else
    tv_ptr = 0;

  if(netPath->eventSock > netPath->generalSock)
    nfds = netPath->eventSock;
  else
    nfds = netPath->generalSock;

  tv.tv_sec = 3;
  tv.tv_usec = 0;
  tv_ptr = &tv;
  ret = select(0, &readfds, 0, 0, tv_ptr);


#ifdef linux
  if(ret < 0)
  {
    if(errno == EAGAIN || errno == EINTR)
      return 0;
  }
#endif
  return ret;
}

ssize_t netRecvEvent(Octet *buf, TimeInternal *time, NetPath *netPath)
{
/*	
  ssize_t ret;
  struct msghdr msg;
  struct iovec vec[1];
  struct sockaddr_in from_addr;
  union {
      struct cmsghdr cm;
      char control[sizeof(struct timeval)];
  } cmsg_un;
  struct cmsghdr *cmsg;
  struct timeval *tv;
  
  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(struct sockaddr_in);
    
  vec[0].iov_base = buf;
  vec[0].iov_len = PACKET_SIZE;
  
  memset(&msg, 0, sizeof(msg));
  memset(&from_addr, 0, sizeof(from_addr));
  memset(buf, 0, PACKET_SIZE);
  memset(&cmsg_un, 0, sizeof(cmsg_un));
  
  msg.msg_name = (caddr_t)&from_addr;
  msg.msg_namelen = sizeof(from_addr);
  msg.msg_iov = vec;
  msg.msg_iovlen = 1;
  msg.msg_control = cmsg_un.control;
  msg.msg_controllen = sizeof(cmsg_un.control);
  msg.msg_flags = 0;

*/
  int ret;
  struct sockaddr_in addr;
  struct timeval tv;

  socklen_t addr_len = sizeof(struct sockaddr_in);

  
  getTime( time );            /* old fashion */

 // ret = recvmsg(netPath->eventSock, &msg, MSG_DONTWAIT);
 // ret = recvfrom(netPath->eventSock, (char*)&msg, PACKET_SIZE, 0, (struct sockaddr *)&addr, &addr_len);
  ret = recvfrom(netPath->eventSock, buf, PACKET_SIZE, 0, (struct sockaddr *)&addr, &addr_len);
	DBGV("receive event\n");

  if(ret <= 0)
  {
    
    return 0;
  }
#if 0 //not supported Windows  
  if(msg.msg_flags&MSG_TRUNC)
  {
    ERROR("received truncated message\n");
    return 0;
  }
  
  /* get time stamp of packet */
  if(!time)
  {
    ERROR("null receive time stamp argument\n");
    return 0;
  }
  
  if(msg.msg_flags&MSG_CTRUNC)
  {
    ERROR("received truncated ancillary data\n");
    return 0;
  }
  
  if(msg.msg_controllen < sizeof(cmsg_un.control))
  {
    ERROR("received short ancillary data (%d/%d)\n",
      msg.msg_controllen, (int)sizeof(cmsg_un.control));
    
    return 0;
  }
 
  tv = 0;

  for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg))
  {
    if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_TIMESTAMP)
      tv = (struct timeval *)CMSG_DATA_WIN(cmsg);
  }

  
  if(tv)
  {
    time->seconds = tv->tv_sec;
    time->nanoseconds = tv->tv_usec*1000;
    DBGV("kernel recv time stamp %us %dns\n", time->seconds, time->nanoseconds);
  }
  else
  {
    /* do not try to get by with recording the time here, better to fail
       because the time recorded could be well after the message receive,
       which would put a big spike in the offset signal sent to the clock servo */
    DBG("no recieve time stamp\n");
    return 0;
  }
#endif

  return ret;


#if 0 //__ADSPBF518__
  int ret;
  struct sockaddr_in addr;
  struct timeval tv;

  socklen_t addr_len = sizeof(struct sockaddr_in);

  /* get the recv time stamp */

  getTime( time );            /* old fashion */
  
  ret = recvfrom(netPath->eventSock, buf, PACKET_SIZE, MSG_DONTWAIT, (struct sockaddr *)&addr, &addr_len);
	//test with blocking socket ZC
 // ret = recvfrom(netPath->eventSock, buf, PACKET_SIZE, 0, (struct sockaddr *)&addr, &addr_len);
	DBGV("receive event\n");
	
  if( ret <= 0 )
      return 0;         /* return -1 is acutally ignored */
 
  unsigned int retNumberOfSeconds;
  unsigned int retNumberOfNanoSeconds;
  unsigned int overflowCount;
  unsigned int sequenceId;
  unsigned int messageType;
  unsigned int hashValue;

  if( addr.sin_addr.s_addr != inet_addr( myIP ) ) /* the message is from other node */
  {
      while( 1 )
      {
          if( !getRxStamp( &retNumberOfSeconds, &retNumberOfNanoSeconds, &overflowCount, &sequenceId, &messageType, &hashValue ) )
          {
              ERROR ( "Get a message, but no recv time stamp, use system time, %d:%d\n", time->seconds, time->nanoseconds );
              break;
          }
          if( !checkMsgStamp( buf, sequenceId, messageType ) )
          {
              DBGV( "Get message and timestamp, but no mathch, get next\n" );
          }
          else
          {
              time->seconds = retNumberOfSeconds;
              time->nanoseconds = retNumberOfNanoSeconds;
              break;
          }
      }
      DBGV( "Get an event message from other nodes at %ds:%dns\n", time->seconds, time->nanoseconds );
  }
  else
  {
      DBGV( "Get an event message from self nodes at %ds:%dns\n", time->seconds, time->nanoseconds );
  }

  return ret;
#endif  
}

ssize_t netRecvGeneral(Octet *buf, NetPath *netPath)
{
  ssize_t ret;
  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(struct sockaddr_in);
  
  ret = recvfrom(netPath->generalSock, buf, PACKET_SIZE, 0, (struct sockaddr *)&addr, &addr_len);
  if(ret == SOCKET_ERROR)
  {
      return 0;
  }
  
  return ret;
}

ssize_t netSendEvent(Octet *buf, UInteger16 length, NetPath *netPath)
{
  ssize_t ret;
  struct sockaddr_in addr;
  
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PTP_EVENT_PORT);
  addr.sin_addr.s_addr = netPath->multicastAddr;
  
  ret = sendto(netPath->eventSock, buf, length, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if(ret <= 0)
    DBG("error sending multi-cast event message\n");
  
  if(netPath->unicastAddr)
  {
    addr.sin_addr.s_addr = netPath->unicastAddr;
    
    ret = sendto(netPath->eventSock, buf, length, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if(ret <= 0)
      DBG("error sending uni-cast event message\n");
      
     /* lwIP 1.3.0 multicast does not support IP_MULTICAST_LOOP  */
     /* Master cannot receive the sync message sent by itself */
     /* to walkaround this issue, send an extra unicast message to itself */
  	 /* self-message for unicasting */
/*    addr.sin_addr.s_addr = inet_addr( myIP );            
    ret = sendto(netPath->eventSock, buf, length, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if(ret <= 0)
      DBG("error sending to self event message\n");
    */  
      
  }
  
  return ret;
}

ssize_t netSendGeneral(Octet *buf, UInteger16 length, NetPath *netPath)
{
  ssize_t ret;
  struct sockaddr_in addr;
  
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PTP_GENERAL_PORT);
  addr.sin_addr.s_addr = netPath->multicastAddr;
  
  ret = sendto(netPath->generalSock, buf, length, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if(ret <= 0)
    DBG("error sending multi-cast general message\n");
  
  if(netPath->unicastAddr)
  {
    addr.sin_addr.s_addr = netPath->unicastAddr;
    
    ret = sendto(netPath->eventSock, buf, length, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if(ret <= 0)
      DBG("error sending uni-cast general message\n");
  }
  
  return ret;
}

