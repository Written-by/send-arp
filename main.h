#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h> 
#include <sys/socket.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ethhdr.h"
#include <iostream>
using namespace std;

struct EthArpPacket final{
	EthHdr eth_;
	ArpHdr arp_;
};

void usage(){
	printf("syntax : send-arp <interface> <sender ip> <target ip>\n");
	printf("sample : send-arp wlan0 192.168.10.2 192.168.10.1\n");
}

int GetInterfaceMacAddress(const char *ifname, char *mac){
    uint8_t *mac_addr;
    struct ifreq ifr;
    int sockfd, ret;

    sockfd=socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd<0){
        perror("sockfd");
        return -1;
    }
    
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ret=ioctl(sockfd, SIOCGIFHWADDR, &ifr);  
    if(ret<0){
      perror("ioctl");
      close(sockfd);
      return -1;
    }
    mac_addr=(uint8_t *)(ifr.ifr_hwaddr.sa_data); 
    close(sockfd);

    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    return 0;
}

int GetInterfaceIPAddress(const char *ifname, char *ip){
    struct ifreq ifr;
    int sockfd, ret;
    char ip_addr[40];

    sockfd=socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd<0){
        perror("sockfd");
        return -1;
    }

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ret=ioctl(sockfd, SIOCGIFADDR, &ifr);
    if(ret<0){
      perror("ioctl");
      close(sockfd);
      return -1;
    }
    close(sockfd);
    
    inet_ntop(AF_INET, ifr.ifr_addr.sa_data+2, ip_addr, sizeof(struct sockaddr));
    sprintf(ip, "%s", ip_addr); 
    return 0;
}

int make_packet(struct EthArpPacket *packet, char *ethdmac, char *ethsmac, int op, char *arpsmac, char *arpsip, char *arptmac, char *arptip){
    packet->eth_.dmac_=Mac(ethdmac);
    packet->eth_.smac_=Mac(ethsmac);
    packet->eth_.type_=htons(EthHdr::Arp);
    packet->arp_.hrd_=htons(ArpHdr::ETHER);
    packet->arp_.pro_=htons(EthHdr::Ip4);
    packet->arp_.hln_=Mac::SIZE;
    packet->arp_.pln_=Ip::SIZE;
    packet->arp_.op_=htons(op);
    packet->arp_.smac_=Mac(arpsmac);
    packet->arp_.sip_=htonl(Ip(arpsip));
    packet->arp_.tmac_=Mac(arptmac);
    packet->arp_.tip_=htonl(Ip(arptip));
    return 0;
}

bool is_Reply(struct EthHdr *ethhdr,struct ArpHdr *arphdr, char *memac,char *meip, char *vip){
    if(ntohs(ethhdr->type_)!=0x0806) return false;
    if(ethhdr->dmac_!=Mac(memac)||arphdr->op()!=ArpHdr::Reply||arphdr->sip_!=htonl(Ip(vip))||arphdr->tmac_!=Mac(memac)||arphdr->tip_!=htonl(Ip(meip))){
            printf("wrong ARP");
            return false;
        }
    return true;
}
