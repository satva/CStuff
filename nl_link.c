#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/if.h>

#define ERR_RET(x) do { perror(x); return EXIT_FAILURE; } while (0);
#define BUFFER_SIZE 4095

int  loop (int sock, struct sockaddr_nl *addr)
{
    int     received_bytes = 0;
    struct  nlmsghdr *nlh;
//    char    iname[5];
    struct ifinfomsg *ifinfo;	
//    struct  rtmsg *route_entry;  /* This struct represent a route entry \
                                    in the routing table */
    struct  rtattr *route_attribute; /* This struct contain route \
                                            attributes (route type) */
    int     route_attribute_len = 0;
    char    buffer[BUFFER_SIZE];

    bzero(buffer, sizeof(buffer));

    /* Receiving netlink socket data */
    while (1)
    {
        received_bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (received_bytes < 0)
            ERR_RET("recv");
        /* cast the received buffer */
        nlh = (struct nlmsghdr *) buffer;
        /* If we received all data ---> break */
        if (nlh->nlmsg_type == NLMSG_DONE)
	{
            break;
	}
        /* We are just intrested in Routing information */
        if (addr->nl_groups == RTMGRP_LINK)
	{
            break;
	}
    }

    /* Reading netlink socket data */
    /* Loop through all entries */
    /* For more informations on some functions :
     * http://www.kernel.org/doc/man-pages/online/pages/man3/netlink.3.html
     * http://www.kernel.org/doc/man-pages/online/pages/man7/rtnetlink.7.html
     */

    for ( ; NLMSG_OK(nlh, received_bytes); \
                    nlh = NLMSG_NEXT(nlh, received_bytes))
    {
	printf("%d bytes received\n",received_bytes);
	
        ifinfo = (struct ifinfomsg *) NLMSG_DATA(nlh);
	route_attribute = IFLA_RTA(ifinfo);
	route_attribute_len = IFLA_PAYLOAD(nlh);
	printf(" flags:0x%x\n",ifinfo->ifi_flags);
	for ( ; RTA_OK(route_attribute, route_attribute_len); \
            route_attribute = RTA_NEXT(route_attribute, route_attribute_len))
        {	
	printf("rtattr Type: %d\n",route_attribute->rta_type);
        /* Get the route data */
/*
	printf("=========================\n");
	printf("message Type:%u\n",nlh->nlmsg_type);
	printf("family:%c\n",ifinfo->ifi_family);
	printf("   pad:%c\n",ifinfo->__ifi_pad);
	printf("  type:%u\n",ifinfo->ifi_type);
	printf(" index:%d\n",ifinfo->ifi_index);
	printf("change:0x%x\n",ifinfo->ifi_change);
	if (nlh->nlmsg_type == RTM_DELLINK)
            printf("Deleting Link\n");
	if (nlh->nlmsg_type == RTM_NEWLINK)
            printf("New Link\n");
*/
/*	
	if (route_attribute->rta_type == IFLA_IFNAME)
	{
		//strncpy(iname,RTA_DATA(route_attribute),sizeof(iname));
		printf("%s\n",RTA_DATA(route_attribute));
		if ( ifinfo->ifi_flags & 0x1 == 0x1 )
		printf("up\n");
		else
		printf("down\n");	
	}
*/	
     }
  }

    return 0;
}

int main(int argc, char **argv)
{
    int sock = -1;
    struct sockaddr_nl addr;

    /* Zeroing addr */
    bzero (&addr, sizeof(addr));

    if ((sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
        ERR_RET("socket");

    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_LINK;

    if (bind(sock,(struct sockaddr *)&addr,sizeof(addr)) < 0)
        ERR_RET("bind");

    while (1)
        loop (sock, &addr);

    /* Close socket */
    close(sock);

    return 0;
}

