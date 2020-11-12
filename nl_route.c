#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <unistd.h>

#define ERR_RET(x) do { perror(x); return EXIT_FAILURE; } while (0);
#define BUFFER_SIZE 4095
#define NIPQUAD(addr) \
        ((unsigned char *)&addr)[0], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[3]


int main(int argc, char **argv)
{
    int sock = -1;
    struct sockaddr_nl addr;
    int     received_bytes = 0;
    struct  nlmsghdr *nlh;
    char    destination_address[32];
    char    gateway_address[32];
    char ifname[16];
    struct  rtmsg *route_entry;                                    
    struct  rtattr *route_attribute;                                            
    int     route_attribute_len = 0;
    char    buffer[BUFFER_SIZE];
    bzero(ifname,sizeof(ifname));
    bzero(destination_address, sizeof(destination_address));
    bzero(gateway_address, sizeof(gateway_address));
    bzero(buffer, sizeof(buffer));
    bzero (&addr, sizeof(addr));

    if ((sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
        ERR_RET("socket");

    addr.nl_family = AF_NETLINK;
//    addr.nl_pid = getpid();
    addr.nl_groups = RTMGRP_IPV4_ROUTE;

    if (bind(sock,(struct sockaddr *)&addr,sizeof(addr)) < 0)
        ERR_RET("bind");

    while (1)
    {
        received_bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (received_bytes < 0)
            ERR_RET("recv");
        nlh = (struct nlmsghdr *) buffer;
	printf("nlh->nlmsg_len %u\n",nlh->nlmsg_len);
	printf("nlh->nlmsg_type %u\n",nlh->nlmsg_type);
	printf("nlh->nlmsg_flags %u\n",nlh->nlmsg_flags);
    	printf("nlh->nlmsg_seq %u\n",nlh->nlmsg_seq);
    	printf("nlh->nlmsg_pid %u\n",nlh->nlmsg_pid);
	
        for ( ; NLMSG_OK(nlh, received_bytes);nlh = NLMSG_NEXT(nlh, received_bytes))
    	{	
        	route_entry = (struct rtmsg *) NLMSG_DATA(nlh);
		printf("route_entry->rtm_family %u\n",route_entry->rtm_family);
		printf("route_entry->rtm_dst_len %u\n",route_entry->rtm_dst_len);
		printf("route_entry->rtm_src_len %u\n",route_entry->rtm_src_len);
		printf("route_entry->rtm_tos %u\n",route_entry->rtm_tos);
		printf("route_entry->rtm_table %u\n",route_entry->rtm_table);
		printf("route_entry->rtm_protocol %u\n",route_entry->rtm_protocol);
		printf("route_entry->rtm_scope %u\n",route_entry->rtm_scope);
		printf("route_entry->rtm_type %u\n",route_entry->rtm_type);
		printf("route_entry->rtm_flags %u\n",route_entry->rtm_flags);
	       	route_attribute = (struct rtattr *) RTM_RTA(route_entry);
	        route_attribute_len = RTM_PAYLOAD(nlh);
        	for ( ; RTA_OK(route_attribute, route_attribute_len);route_attribute = RTA_NEXT(route_attribute, route_attribute_len))
        	{
			switch(route_attribute->rta_type)
			{
				case RTA_DST:
					printf("RTA_DST: ");
					printf("%u.%u.%u.%u\n",NIPQUAD(*(char*)RTA_DATA(route_attribute)));	
				break;
				case RTA_SRC:
					printf("RTA_SRC:\n");
				break;
				case RTA_IIF:
					printf("RTA_IIF:\n");
				break;
				case RTA_OIF:
					printf("RTA_OIF: ");
					printf("%d\n",*((char*)RTA_DATA(route_attribute)));
				break;
				case RTA_GATEWAY:
					printf("RTA_GATEWAY: ");
					printf("%u.%u.%u.%u\n",NIPQUAD(*(char*)RTA_DATA(route_attribute)));	
				break;
				case RTA_PRIORITY:
					printf("RTA_PRIORITY:\n");
				break;
				case RTA_PREFSRC:
					printf("RTA_PREFSRC:\n");
				break;
				case RTA_METRICS:
					printf("RTA_METRICS:\n");
				break;
				case RTA_MULTIPATH:
					printf("RTA_MULTIPATH:\n");
				break;
				case RTA_PROTOINFO:
					printf("RTA_PROTOINFO:\n");
				break;
				case RTA_FLOW:
					printf("RTA_FLOW:\n");
				break;
				case RTA_CACHEINFO:
					printf("RTA_CACHEINFO:\n");
				break;
				case RTA_SESSION:
					printf("RTA_SESSION:\n");
				break;
				case RTA_MP_ALGO:
					printf("RTA_MP_ALGO:\n");
				break;
				case RTA_TABLE:
					printf("RTA_TABLE: ");
					printf("%d\n",0xFF & (*((char*)RTA_DATA(route_attribute))));
				break;
				default:
					printf("Attr Type %d\n",route_attribute->rta_type);
			}
        	}

    }



    }
    /* Close socket */
    close(sock);

    return 0;
}

