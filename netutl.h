#pragma once
#include "lib.c"
#include "modules/hashmap.h"
#include "fs.h"

uint16_t flip_short(uint16_t short_int)
{
    uint32_t first_byte = *((uint8_t *)(&short_int));
    uint32_t second_byte = *((uint8_t *)(&short_int) + 1);
    return (first_byte << 8) | (second_byte);
}

uint32_t flip_long(uint32_t long_int)
{
    uint32_t first_byte = *((uint8_t *)(&long_int));
    uint32_t second_byte = *((uint8_t *)(&long_int) + 1);
    uint32_t third_byte = *((uint8_t *)(&long_int) + 2);
    uint32_t fourth_byte = *((uint8_t *)(&long_int) + 3);
    return (first_byte << 24) | (second_byte << 16) | (third_byte << 8) | (fourth_byte);
}

uint16_t htons(uint16_t hostshort)
{
    return flip_short(hostshort);
}

uint32_t htonl(uint32_t hostlong)
{
    return flip_long(hostlong);
}

uint16_t ntohs(uint16_t netshort) 
{
    return flip_short(netshort);
}

uint32_t ntohl(uint32_t netlong) 
{
    return flip_long(netlong);
}

struct dhcppack {
    uint8_t op;
    uint8_t hardware_type;
    uint8_t hardware_addr_len;
    uint8_t hops;
    uint32_t xid;
    uint16_t seconds;
    uint16_t flags;     // may be broken
    uint32_t client_ip; //
    uint32_t your_ip;
    uint32_t server_ip;
    uint32_t gateway_ip;
    uint8_t client_hardware_addr[16];
    uint8_t server_name[64];
    uint8_t file[128];
    uint8_t options[64];
};

struct ipv4_packet {
	uint8_t  version_ihl;
	uint8_t  dscp_ecn;
	uint16_t length;
	uint16_t ident;
	uint16_t flags_fragment;
	uint8_t  ttl;
	uint8_t  protocol;
	uint16_t checksum;
	uint32_t source;
	uint32_t destination;
	uint8_t  payload[];
} __attribute__ ((packed)) __attribute__((aligned(2)));

struct icmp_header {
	uint8_t type;
	uint8_t code;
	uint16_t csum;
	uint16_t rest_of_header;
	uint8_t data[];
} __attribute__((packed)) __attribute__((aligned(2)));

struct udp_packet {
	uint16_t source_port;
	uint16_t destination_port;
	uint16_t length;
	uint16_t checksum;
	uint8_t  payload[];
} __attribute__ ((packed)) __attribute__((aligned(2)));

struct tcp_header {
	uint16_t source_port;
	uint16_t destination_port;

	uint32_t seq_number;
	uint32_t ack_number;

	uint16_t flags;
	uint16_t window_size;
	uint16_t checksum;
	uint16_t urgent;

	uint8_t  payload[];
} __attribute__((packed)) __attribute__((aligned(2)));

struct tcp_check_header {
	uint32_t source;
	uint32_t destination;
	uint8_t  zeros;
	uint8_t  protocol;
	uint16_t tcp_len;
	uint8_t  tcp_header[];
};

struct tcp_socket {
	uint16_t port;
	struct sockaddr dest;
	uint32_t priv32[4];

	size_t unread;
	char * buf;
	int nonblocking;
};

static uint16_t icmp_checksum(struct ipv4_packet * packet) {
	uint32_t sum = 0;
	uint16_t * s = (uint16_t *)packet->payload;
	for (int i = 0; i < (ntohs(packet->length) - 20) / 2; ++i) {
		sum += ntohs(s[i]);
	}
	if (sum > 0xFFFF) {
		sum = (sum >> 16) + (sum & 0xFFFF);
	}
	return ~(sum & 0xFFFF) & 0xFFFF;
}

uint16_t calculate_ipv4_checksum(struct ipv4_packet * p) {
	uint32_t sum = 0;
	uint16_t * s = (uint16_t *)p;

	/* TODO: Checksums for options? */
	for (int i = 0; i < 10; ++i) {
		sum += ntohs(s[i]);
		if (sum > 0xFFFF) {
			sum = (sum >> 16) + (sum & 0xFFFF);
		}
	}
	return ~(sum & 0xFFFF) & 0xFFFF;
}

hashtable __sockets;

void net_add_socket(int id)
{
	ext2_inode* inode = (ext2_inode*)kalloc(sizeof(ext2_inode), KERN_MEM);
	inode->type = INODE_TYPE_SOCKET;
	inode->gen_no = id;
	hashmap_set(&__sockets, (void*)id, inode);
}

void net_remove_socket(int id)
{
	free(hashmap_get(&__sockets, (void*)id));
	hashmap_remove(&__sockets, (void*)id);
}