#pragma once
#include "netutl.h"
#include "port.h"
#include "mem.h"
#define ROK     (1<<0)
#define RER     (1<<1)
#define TOK     (1<<2)
#define NPORT_DNS          53
#define NPORT_BOOTP_SERVER 67
#define NPORT_BOOTP_CLIENT 68
#define NPORT_NTP          123
#define NPORT_OSHELPER    4950

#define REG_EEPROM 0x0014
#define NET_PORT 0x360
#define NET_PORTMAX 0x367
#define NET_INTFACE 0x300
#define MAIN_PORT 80

#define HTTP_OK 200
#define HTTP_BADREQ 400
#define HTTP_NOTFOUND 404

#define AF_UNSPEC	0
#define AF_UNIX		1	
#define AF_INET		2	
#define AF_AX25		3	
#define AF_IPX		4	
#define AF_APPLETALK 5	
#define	AF_NETROM	6	
#define AF_BRIDGE	7	
#define AF_AAL5		8	
#define AF_X25		9	
#define AF_INET6	10	
#define AF_MAX		12	

#define PF_UNSPEC	AF_UNSPEC
#define PF_UNIX		AF_UNIX
#define PF_INET		AF_INET
#define PF_AX25		AF_AX25
#define PF_IPX		AF_IPX
#define PF_APPLETALK	AF_APPLETALK
#define	PF_NETROM	AF_NETROM
#define PF_BRIDGE	AF_BRIDGE
#define PF_AAL5		AF_AAL5
#define PF_X25		AF_X25
#define PF_INET6	AF_INET6

#define RX_BUF_SIZE 8192
#define RX_READ_POINTER_MASK (~3)
#define ETHERNET_TYPE_ARP 0x0806
#define ETHERNET_TYPE_IP  0x0800
#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139

struct rtldev {
    uint8_t bar_type;
    uint16_t io_base;
    uint32_t mem_base;
    int eeprom_exist;
    uint8_t mac_addr[6];
    char *rx_buffer;
    int tx_cur;
} rtl_device;

struct ethframe {

};

int TSAD_array[16];
size_t mem_base, io_base, bar_type;
size_t current_packet_ptr, CAPR;

void rtl8139_send_packet(void *data, size_t len)
{
    // First, copy the data to a physically contiguous chunk of memory
    void *transfer_data = kalloc(len,USER_MEM);
    void *phys_addr = transfer_data;
    memcpy(transfer_data, data, len);

    // Second, fill in physical address of data, and length
    outports(rtl_device.io_base + TSAD_array[rtl_device.tx_cur], (size_t)phys_addr);
    outports(rtl_device.io_base + TSAD_array[rtl_device.tx_cur++], len);
    if (rtl_device.tx_cur > 3)
        rtl_device.tx_cur = 0;
}

typedef struct arpent {
    uint32_t ip_addr;
    uint64_t mac_addr;
};

struct macaddr {
    uint8_t values[6];
};

struct webreq {
   int code;
   int type;//GET is 0 and POST is 1
   char* header;
   size_t size;
};

struct sockaddr {
   uint16_t family;	
   char data[14];	
};

uint8_t broadcast_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

void receive_packet()
{
    uint16_t *t = (uint16_t *)(rtl_device.rx_buffer + current_packet_ptr);
    uint16_t packet_length = *(t + 1);

    t = t + 2;
    void *packet = kalloc(packet_length, USER_MEM);
    memcpy(packet, t, packet_length);
    nethandle(packet, packet_length);

    current_packet_ptr = (current_packet_ptr + packet_length + 4 + 3) & RX_READ_POINTER_MASK;

    if (current_packet_ptr > RX_BUF_SIZE)
        current_packet_ptr -= RX_BUF_SIZE;

    outports(rtl_device.io_base + CAPR, current_packet_ptr - 0x10);
}

void nethandle(void* packet, int len){
  uint16_t status = inports(rtl_device.io_base + 0x3e);

  if(status & TOK) {
        
  }
  if (status & ROK) {
        //kprint("Received packet\n");
  }

  outports(rtl_device.io_base + 0x3E, 0x5);
}


void netwritecmd(uint16_t p_address, size_t p_value)
{
    if (bar_type == 0)
    {
        write32((void*)(mem_base + p_address), p_value);
    }
    else
    {
        outports(io_base, p_address);
        outports(io_base + 4, p_value);
    }
}

size_t netreadcmd(uint16_t p_address)
{
    if (bar_type == 0)
    {
        return read32((void*)(mem_base + p_address));
    }
    else
    {
        outports(io_base, p_address);
        return inports(io_base + 4);
    }
}

int eeprom_detect()
{
    uint32_t val = 0;
    int eeprom_exists = 0;
    netwritecmd(REG_EEPROM, 0x1); 
 
    for(int i = 0; i < 1000 && ! eeprom_exists; i++)
    {
            val = netreadcmd(REG_EEPROM);
            if(val & 0x10)
                    eeprom_exists = 1;
            else
                    eeprom_exists = 0;
    }
    return eeprom_exists;
}


typedef struct {
    uint16_t io_base;
    uint32_t mem_base;
    uint8_t is_e;
} e1000info;

e1000info* global_e1000;
#define E1000_DEV               0x100E
#define E1000_REG_CTRL 		    0x0000
#define E1000_REG_EEPROM 		0x0014
#define E1000_REG_IMASK 		0x00D0

void e1000write_l(e1000info *e, uint16_t addr, uint32_t val)
{
	outportl (e->io_base, addr);
	outportl (e->io_base + 4, val);
}

void e1000write_b(e1000info *e, uint16_t addr, uint32_t val)
{
	outportl (e->io_base, addr);
	outports (e->io_base + 4, val);
}

uint32_t e1000read_l(e1000info *e, uint16_t addr)
{
	outportl (e->io_base, addr);
	return inportl (e->io_base + 4);
}

uint32_t e1000readprom(e1000info *e, uint8_t addr)
{
	uint32_t val = 0;
	uint32_t test;
	if(e->is_e == 0)
		test = addr << 8;
	else
		test = addr << 2;

	e1000write_l(e, E1000_REG_EEPROM, test | 0x1);
	if(e->is_e == 0)
		while(!((val = e1000read_l(e, E1000_REG_EEPROM)) & (1<<4)))
		;//	printf("is %i val %x\n",e->is_e,val);
	else
		while(!((val = e1000read_l(e, E1000_REG_EEPROM)) & (1<<1)))
		;//	printf("is %i val %x\n",e->is_e,val);
	val >>= 16;
	return val;
}

void e1000getmac(e1000info *e, char *mac)
{
	uint32_t temp;
	temp = e1000readprom(e, 0);
	mac[0] = temp &0xff;
	mac[1] = temp >> 8;
	temp = e1000readprom(e, 1);
	mac[2] = temp &0xff;
	mac[3] = temp >> 8;
	temp = e1000readprom(e, 2);
	mac[4] = temp &0xff;
	mac[5] = temp >> 8;
}

void e1000enableint(e1000info *e)
{
	e1000write_l(e, E1000_REG_IMASK ,0x1F6DC);
	e1000write_l(e, E1000_REG_IMASK ,0xff & ~4);
	e1000read_l(e, 0xc0);
}

void e1000setup()
{
    uint32_t mac;
    e1000getmac(global_e1000, (char*)&mac);
    e1000enableint(global_e1000);
}