/*****************************************************************************************************
* GP32 SDK Version 1.1 header file : gpnetlib.h                                                      *
*		The latest patch : 22,May,2001                                                             *
*		SDK Developer : Jstar, Achi                                                                *
*		related library : gpnet.alf                                                                *
*		COPYRIGHT DESCRIPTION														 *
*			The copyright of the source is reserved by GAMEPARK,Inc!!!                            *
*****************************************************************************************************/

#ifndef __GPNETLIB_H__
#define __GPNETLIB_H__

#include "gpos_def.h"

#define GPC_ENET_OK			0
#define GPC_ENET_INITFAIL	1
#define GPC_ENET_INVALIDARG 2
#define GPC_ENET_ALREADY_USED	3

#ifndef __GPSOCKETDEFINE
#define __GPSOCKETDEFINE

typedef long SO_HANDLE;

struct sockaddr {
   unsigned short   sa_family;      /* address family */
   char   sa_data[14];      /* up to 14 bytes of direct address */
};

struct in_addr {
   unsigned long s_addr;
};

struct sockaddr_in {
   short   sin_family;
   unsigned short   sin_port;
   struct   in_addr sin_addr;
   char   sin_zero[8];
};

/* Description of data base entry for a single host.  */
struct hostent
{
   char *h_name;         /* Official name of host.  */
   char **h_aliases;     /* Alias list.  */
   int h_addrtype;       /* Host address type.  */
   int h_length;         /* Length of address.  */
   char **h_addr_list;   /* List of addresses from name server.  */
#ifndef h_addr   
#define h_addr h_addr_list[0] /* Address, for backward compatibility.  */
#endif
};

#define  AF_NS       1     /* local to host (pipes, portals) */
#define  AF_INET     2     /* internetwork: UDP, TCP, etc. */

#define   SOCK_STREAM   1      /* stream socket */  
#define   SOCK_DGRAM    2      /* datagram socket */

#define   SO_DEBUG       0x0001      /* turn on debugging info recording */
#define   SO_ACCEPTCONN  0x0002      /* socket has had listen() */         
#define   SO_REUSEADDR   0x0004      /* allow local address reuse */       
#define   SO_KEEPALIVE   0x0008      /* keep connections alive */          
#define   SO_DONTROUTE   0x0010      /* just use interface addresses */    
#define   SO_BROADCAST   0x0020      /* permit sending of broadcast msgs */
#define   SO_USELOOPBACK 0x0040      /* bypass hardware when possible */   
#define   SO_LINGER      0x0080      /* linger on close if data present */ 
#define   SO_OOBINLINE   0x0100      /* leave received OOB data in line */ 

#define SO_SNDBUF    0x1001      /* send buffer size */                      
#define SO_RCVBUF    0x1002      /* receive buffer size */                   
#define SO_SNDLOWAT  0x1003      /* send low-water mark */                   
#define SO_RCVLOWAT  0x1004      /* receive low-water mark */                
#define SO_SNDTIMEO  0x1005      /* send timeout */                          
#define SO_RCVTIMEO  0x1006      /* receive timeout */                       
#define SO_ERROR     0x1007      /* get error status and clear */            
#define SO_TYPE      0x1008      /* get socket type */                       
#define SO_HOPCNT    0x1009      /* Hop count to get to dst   */             
#define SO_MAXMSG    0x1010      /* get TCP_MSS (max segment size) */        
                                            
#define SO_RXDATA    0x1011      /* get count of bytes in sb_rcv */          
#define SO_MYADDR    0x1012      /* return my IP address */                  
#define SO_NBIO      0x1013      /* set socket into NON-blocking mode */     
#define SO_BIO       0x1014      /* set socket into blocking mode */         
#define SO_NONBLOCK  0x1015      /* set/get blocking mode via optval param */
#define SO_CALLBACK  0x1016      /* set/get zero_copy callback routine  */    
                                            
#define   MSG_OOB      	0x1      /* process out-of-band data */              
#define   MSG_PEEK   	0x2      /* peek at incoming message */              
#define   MSG_DONTROUTE 0x4      /* send without using routing tables */     
#define   MSG_NEWPIPE   0x8      /* New pipe for recvfrom call   */          
#define   MSG_EOR      	0x10      /* data completes record */                
#define   MSG_DONTWAIT  0x20      /* this message should be nonblocking */   

/* BSD sockets errors */
#ifndef _GPSOCK_ERR_DEF
#define _GPSOCK_ERR_DEF
#define   ENOBUFS       1
#define   ETIMEDOUT     2
#define   EISCONN       3
#define   EOPNOTSUPP    4
#define   ECONNABORTED  5
#define   EWOULDBLOCK   6
#define   ECONNREFUSED  7
#define   ECONNRESET    8
#define   ENOTCONN      9
#define   EALREADY      10
#define   EINVAL        11
#define   EMSGSIZE      12
#define   EPIPE         13
#define   EDESTADDRREQ  14
#define   ESHUTDOWN     15
#define   ENOPROTOOPT   16
#define   EHAVEOOB      17
#define   ENOMEM        18
#define   EADDRNOTAVAIL 19
#define   EADDRINUSE    20
#define   EAFNOSUPPORT  21
#define   EINPROGRESS   22
#define   ELOWER        23 /* lower layer (IP) error */
#define   EIEIO 27 /* bad input/output on Old Macdonald's farm :-) */
#endif
#endif	/*__GPSOCKDEFINE */


#ifndef TPS
#define TPS 50L					   /* DON'T CHANGE TPS VALUE */
#endif

#include "gpsockdef.h"

void GpNetTicker(void);
int GpNetInit(int net_tps);
//void GpNetBackloop(void);
#define GpNetBackloop()	
int GpNetParseIp(char * host, unsigned int * m_ip);

//=========================================================================
long GpSockCreate(int family,int type,int protocol);
int GpSockConnect(long so,struct sockaddr_in * s_addr);
int GpSockSend(long so, char *msg, int len, int flags);
int GpSockSendto(long so,char *msg,int len, int flags,struct sockaddr_in *s_addr);
int GpSockRecv(long so, char *msg, int len, int flags);
int GpSockRecvfrom(long so,char *msg,int len,int flags,struct sockaddr_in *s_addr);
int GpSockClose(long so);
int GpSockShutdown(long so,int how);
int GpPPPShutdown(void);
int GpSockPeerGet(long so,struct sockaddr_in *peer);
int GpSockNameGet(long so,struct sockaddr_in *name);
int GpSockOptGet(long so,int optname,void *optval);
int GpSockOptSet(long so,int optname,void *optval);
int GpSockConnected(long s);

#endif /*__GPNETLIB_H__*/
