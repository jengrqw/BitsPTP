/* net.c */

#include "../ptpd.h"


extern char hwaddr[6];
extern  char myIP[16];




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
  
  return PTP_DEFAULT;
}

UInteger32 findIface(Octet *ifaceName, UInteger8 *communicationTechnology,
  Octet *uuid, NetPath *netPath)
{


    memcpy( uuid, hwaddr, PTP_UUID_LENGTH );     
    *communicationTechnology = PTP_ETHER;

    memcpy(ifaceName, IFACE_NAME, IFACE_NAME_LENGTH);        
    return TRUE;        /* Ture successful return */



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
  
  DBG("netInit\n");
  
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

    DBG( "Unicast address: %s\n", rtOpts->unicastAddress );
    if( inet_addr( (char*)rtOpts->unicastAddress )  == -1  )  
    {
      ERROR("failed to encode uni-cast address: %s\n", rtOpts->unicastAddress);
      return FALSE;
    }
    netAddr.s_addr = inet_addr( (char*)rtOpts->unicastAddress );
    netPath->unicastAddr = netAddr.s_addr;
  }
  else
    netPath->unicastAddr = 0;
  
  /* resolve PTP subdomain */
  if(!lookupSubdomainAddress(rtOpts->subdomainName, (Octet *)addrStr))
    return FALSE;

  if( inet_addr( addrStr)  == -1 )
  {
    ERROR("failed to encode multi-cast address: %s\n", addrStr);
    return FALSE;
  }
  
  netAddr.s_addr = inet_addr( addrStr);
  netPath->multicastAddr = netAddr.s_addr;
  
  s = addrStr;
  for(i = 0; i < SUBDOMAIN_ADDRESS_LENGTH; ++i)
  {
    ptpClock->subdomain_address[i] = strtol(s, &s, 0);
    
    if(!s)
      break;
    
    ++s;
  }
  
  /* multicast send only on specified interface */
  imr.imr_multiaddr.s_addr = netAddr.s_addr;
  imr.imr_interface.s_addr = htonl(INADDR_ANY);;
  
  /* join multicast group (for receiving) on specified interface */
  if( setsockopt(netPath->eventSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(struct ip_mreq))  < 0
    || setsockopt(netPath->generalSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(struct ip_mreq)) < 0 )
  {
    PERROR("failed to join the multi-cast group\n");
    return FALSE;
  }
    
  return TRUE;
}

/* shut down the UDP stuff */
Boolean netShutdown(NetPath *netPath)
{
  struct ip_mreq imr;

  imr.imr_multiaddr.s_addr = netPath->multicastAddr;
  imr.imr_interface.s_addr = htonl(INADDR_ANY);

  setsockopt(netPath->eventSock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &imr, sizeof(struct ip_mreq));
  setsockopt(netPath->generalSock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &imr, sizeof(struct ip_mreq));
  
  netPath->multicastAddr = 0;
  netPath->unicastAddr = 0;
  
  if(netPath->eventSock > 0)
    close(netPath->eventSock);
  netPath->eventSock = -1;
  
  if(netPath->generalSock > 0)
    close(netPath->generalSock);
  netPath->generalSock = -1;
    
  return TRUE;
}

int netSelect(TimeInternal *timeout, NetPath *netPath)
{
  int ret, nfds;
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
  ret = select(nfds + 1, &readfds, 0, 0, tv_ptr);


  return ret;
}

ssize_t netRecvEvent(Octet *buf, TimeInternal *time, NetPath *netPath)
{

  int ret;
  struct sockaddr_in addr;
  struct timeval tv;

  socklen_t addr_len = sizeof(struct sockaddr_in);

  /* get the recv time stamp */

  getTime( time );            /* old fashion */
  
  ret = recvfrom(netPath->eventSock, buf, PACKET_SIZE, MSG_DONTWAIT, (struct sockaddr *)&addr, &addr_len);
  if( ret <= 0 )
      return 0;         /* return -1 is acutally ignored */
 
  DBGV("receive event\n");
 
  unsigned int retNumberOfSeconds;
  unsigned int retNumberOfNanoSeconds;
  UInteger16 overflowCount;
  UInteger16 sequenceId;
  UInteger16 messageType;
  UInteger16 hashValue;

  if( addr.sin_addr.s_addr != inet_addr( myIP ) ) /* the message is from other node */
  {
      while( 1 )
      {
          if( !getRxStamp( &retNumberOfSeconds, &retNumberOfNanoSeconds, &overflowCount, &sequenceId, &messageType, &hashValue ) )
          {
   //          ERROR ( "Get a message, but no recv time stamp, use system time, %d:%d\n", time->seconds, time->nanoseconds );
             DBGV ( "Get a message, but no recv time stamp, use system time, %d:%d\n", time->seconds, time->nanoseconds );
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
//      DBGV( "Get an event message from other nodes at %ds:%dns\n", time->seconds, time->nanoseconds );
      DBGV( "Get an event message  at %ds:%dns\n", time->seconds, time->nanoseconds );
  }
  else
  {
      DBGV( "Get an event message from self nodes at %ds:%dns\n", time->seconds, time->nanoseconds );
  }

  return ret;

}

ssize_t netRecvGeneral(Octet *buf, NetPath *netPath)
{
  int ret;
  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(struct sockaddr_in);
  
  ret = recvfrom(netPath->generalSock, buf, PACKET_SIZE, MSG_DONTWAIT, (struct sockaddr *)&addr, &addr_len);
  if(ret <= 0)
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
    
 }
  
  return ret;
}

