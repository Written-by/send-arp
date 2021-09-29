#pragma once

#include <cstdint>
#include <arpa/inet.h>
#include "mac.h"
#include "ip.h"
#pragma pack(push, 1)

struct ArpHdr final{
	uint16_t hrd_;
	uint16_t pro_;
	uint8_t hln_;
	uint8_t pln_;
	uint16_t op_;
	Mac smac_;
	Ip sip_;
	Mac tmac_;
	Ip tip_;

	uint16_t hrd(){return ntohs(hrd_);}
	uint16_t pro(){return ntohs(pro_);}
	uint8_t hln(){return hln_;}
	uint8_t pln(){return pln_;}
	uint16_t op(){return ntohs(op_);}
	Mac smac(){return smac_;}
	Ip sip(){return ntohl(sip_);}
	Mac tmac(){return tmac_;}
	Ip tip(){return ntohl(tip_);}

	enum: uint16_t{
		NETROM=0,
		ETHER=1,
		EETHER=2,
		AX25=3,
		PRONET=4,
		CHAOS=5,
		IEEE802=6,
		ARCNET=7,
		APPLETLK=8,
		LANSTAR=9,
		DLCI=15,
		ATM=19,
		METRICOM=23,
		IPSEC=31
	};

	enum: uint16_t{
		Request=1,
		Reply=2,
		RevRequest=3,
		RevReply=4,
		InvRequest=8,
		InvReply=9
	};
};
typedef ArpHdr *PArpHdr;
#pragma pack(pop)
