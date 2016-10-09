#ifndef DHCPD_H_
#define DHCPD_H_

//#include "portmacro.h"
#include "lwip/inet.h"
#include "leases.h"
struct dhcpd
{
	unsigned char op;
	unsigned char htype;
	unsigned char hlen;
	unsigned char hops;
	unsigned long xid;
	unsigned short secs;
	unsigned short flags;
	unsigned long ciaddr;
	unsigned long yiaddr;
	unsigned long siaddr;
	unsigned long giaddr;
	unsigned char chaddr[16];
	unsigned char sname[64];
	unsigned char file[128];
	unsigned long cookie;
	unsigned char options[308]; /* 312 - cookie */
};

struct server_config_t
{
	struct in_addr  server;
	struct in_addr  mask;
	struct in_addr  start;
	struct in_addr  end;

	char *interface;		/* The name of the interface to use */
	unsigned char arp[6];		/* Our arp address */
	unsigned long lease;		/* lease time in seconds (host order) */
	unsigned long max_leases; 	/* maximum number of leases (including reserved address) */
	unsigned long auto_time; 	/* how long should udhcpd wait before writing a config file if this is zero, it will only write one on SIGUSR1 */
	unsigned long decline_time;     /* how long an address is reserved if a client returns a decline message */
	unsigned long conflict_time;    /* how long an arp conflict offender is leased for */
	unsigned long offer_time; 	/* how long an offered address is reserved */
	unsigned long min_lease; 	/* minimum lease a client can request*/

	struct in_addr  siaddr;		/* next server bootp option */
	char *sname;				/* bootp server name */
	char *boot_file;			/* bootp boot file option */
};

#endif /* DHCPD_H_ */
