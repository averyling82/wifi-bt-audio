/*
 * leases.h
 *
 * Created: 2013/1/7 14:09:21
 *  Author: Administrator
 */


#ifndef LEASES_H_
#define LEASES_H_

#define LEASE_H_
#include "lwip/inet.h"
struct dhcpOfferedAddr
{
    //struct dhcpOfferedAddr *next;
    unsigned char chaddr[16];
    struct in_addr yiaddr;
    unsigned int expires;   /* all variable is host oder except for address */
    unsigned char *hostname;
    unsigned char flag; /* static lease?*/
};

/* flag */
#define DYNAMICL                0x0
#define STATICL                 0x1
#define DISABLED                0x2
#define RESERVED                0x4
#define DELETED                 0x8

#define SHOW_DYNAMIC            0x1
#define SHOW_STATIC             0x2
#define SHOW_DYNAMIC_STATIC     0x3
#define SHOW_ALL                0x8

#endif /* LEASES_H_ */
