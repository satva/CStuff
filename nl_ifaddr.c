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
    struct nlmsghdr* nlhdr;
    struct ifaddrmsg* ifadr;
    struct rtattr *rta;    
    ssize_t bytes=0;    
    char buffer[BUFFER_SIZE];
    struct req 
    {
    	struct nlmsghdr n;
	struct ifaddrmsg ifc;
    }req;	
    int rtlen = 0;
    char ifname[16];
    char ip[16];
    //typedef struct req req;
    
    /* Zeroing addr */
    bzero (&addr, sizeof(addr));

    if ((sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
        ERR_RET("socket");

    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();

    if (bind(sock,(struct sockaddr *)&addr,sizeof(addr)) < 0)
        ERR_RET("bind");
  
    req.n.nlmsg_len =   NLMSG_LENGTH(sizeof(struct ifaddrmsg));
    req.n.nlmsg_type = RTM_GETADDR;  
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.n.nlmsg_pid = getpid();
    req.n.nlmsg_seq = 0;
  
    req.ifc.ifa_family = AF_INET;
/*    
    rta = (struct rtattr*) ((char*)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    rta->rta_len = RTA_LENGTH(4);
*/	
    bytes = send(sock,&req, req.n.nlmsg_len,0);
    if( bytes < 0 )	
    {
	ERR_RET("Send Failed");
    }
    bytes = recv(sock,buffer,sizeof(buffer),0);
    printf("%d bytes received\n",bytes);
    nlhdr = (struct nlmsghdr*)buffer;    

    printf("nlhdr->nlmsg_len %u\n",nlhdr->nlmsg_len);
/*
    printf("nlhdr->nlmsg_type %u\n",nlhdr->nlmsg_type);
    printf("nlhdr->nlmsg_flags %u\n",nlhdr->nlmsg_flags);
    printf("nlhdr->nlmsg_seq %u\n",nlhdr->nlmsg_seq);
    printf("nlhdr->nlmsg_pid %u\n",nlhdr->nlmsg_pid);
    printf("============================\n");
*/ 
   for(;NLMSG_OK(nlhdr,bytes);nlhdr = NLMSG_NEXT(nlhdr,bytes))
    {
	ifadr = (struct ifaddrmsg*)NLMSG_DATA(nlhdr);
	rta = IFA_RTA(ifadr);
	rtlen = IFA_PAYLOAD(nlhdr);
	printf("rtlen = %d\n",rtlen);
	printf("Link Index: %u\n",ifadr->ifa_index);
	for(; RTA_OK(rta,rtlen);rta = RTA_NEXT(rta,rtlen) )
	{
	//	printf("%d: Type\n",rta->rta_type);
		printf("rt ->len %u\n",rta->rta_len);
		printf("rt ->type %u\n",rta->rta_type);
		switch(rta->rta_type)
		{
			case IFA_UNSPEC:
				printf("IFA_UNSPEC:\n");
			break;
			case IFA_ADDRESS:
				printf("IFA_ADDRESS:");
				inet_ntop(AF_INET,RTA_DATA(rta),ip,sizeof(ip));
				printf("%s\n",ip);
			break;
			case IFA_LOCAL:
				printf("IFA_LOCAL:");
				inet_ntop(AF_INET,RTA_DATA(rta),ip,sizeof(ip));
                                printf("%s\n",ip);
			break;
			case IFA_LABEL:
				printf("IFA_LABEL: ");
				sprintf(ifname,"%s",RTA_DATA(rta));
				printf("%s\n",ifname);
			break;
			case IFA_BROADCAST:
				printf("IFA_BROADCAST:");
				inet_ntop(AF_INET,RTA_DATA(rta),ip,sizeof(ip));
                                printf("%s\n",ip);
			break;	
			case IFA_ANYCAST:
				printf("IFA_ANYCAST:");
			break;
			case IFA_CACHEINFO:
				printf("IFA_CACHEINFO:");
			break;
			case IFA_MULTICAST:
				printf("IFA_MULTICAST:");
			break;
			default:
				printf("None\n");
		}
	}	
	printf("============================\n");
    }
    /* Close socket */
    close(sock);

    return 0;
}

