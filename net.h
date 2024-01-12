#pragma once
#include "netutl.h"
#include "port.h"
#include "smem.h"
#include "pci.h" // to grab the devices!
#include "process.h"
#include "id.h" // Let's use it for port mapping, shall we?
#include "./modules/ext2.h"

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

#define E1000_NUM_RX_DESC 512
#define E1000_NUM_TX_DESC 512
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
ext2_gen_device* netmount; // mount point for Ext2
struct idr* netports;

struct ethframe {
  uint8_t dst_mac_addr[6];
  uint8_t src_mac_addr[6];
  uint16_t type;
  uint8_t data[];
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
void nethandle(void* packet, size_t size)
{
    printf("Got packet with size = %i", size);
}

void rtl8139_receive_packet(uint8_t* buf, size_t* len)
{
    uint16_t *t = (uint16_t *)(rtl_device.rx_buffer + current_packet_ptr);
    uint16_t packet_length = *(t + 1);
    *len = packet_length;

    t = t + 2;
    void *packet = buf;
    memcpy(packet, t, packet_length);

    current_packet_ptr = (current_packet_ptr + packet_length + 4 + 3) & RX_READ_POINTER_MASK;

    if (current_packet_ptr > RX_BUF_SIZE)
        current_packet_ptr -= RX_BUF_SIZE;

    outports(rtl_device.io_base + CAPR, current_packet_ptr - 0x10);
}

static void __rtl_read(struct vfile* vf, char* buffer, size_t size)
{
    if(size == 0)
        memcpy(buffer, rtl_device.mac_addr, 6);
    else
        memcpy(buffer, rtl_device.rx_buffer, size);
}

static void __rtl_write(struct vfile* vf, char* buffer, size_t size)
{
    rtl8139_send_packet(buffer, size);
}

struct vfile* rtl8139_map()
{
    if(rtl_device.rx_buffer == NULL_PTR)
        return (struct vfile*)NULL_PTR;
    struct vfile* vf = (struct vfile*)kalloc(sizeof(struct vfile), KERN_MEM);
    vf->drno = 0x8139;
    vf->name = "/home/dev/rtl8139";
    vf->mem = vmap((void*)rtl_device.mem_base, 0, 0, (struct vfile*)NULL_PTR);
    vf->ops = kalloc(sizeof(struct vfileops), KERN_MEM);
    struct vfileops* ops = (struct vfileops*)vf->ops;
    ops->read = __rtl_read;
    ops->write = __rtl_write;
    return vf;
}

void ip_handle(void* data)
{
    // handle the IP layer
}

void arp_handle(void* data)
{
    // handle the ARP message
}

#define ETHERNET_TYPE_ARP 0x0806
#define ETHERNET_TYPE_IP  0x0800

void __rtl_ext_read(uint8_t* buf, size_t offset, size_t len, ext2_gen_device* dev)
{
    // handle a packet
    size_t len;
    rtl8139_receive_packet(buf, &len);
    struct ethframe* frame = (struct ethframe*)buf;
    if(!memcmp((char*)rtl_device.mac_addr, (char*)frame->dst_mac_addr))
    {
        kprint("Got package!");
    }
    if(frame->type == ETHERNET_TYPE_IP)
        ip_handle(frame->data);
    else if(frame->type == ETHERNET_TYPE_ARP)
        arp_handle(frame->data);
    else    
        kprint("Unknown ethernet type!");
    dev->offset += len; // you can use this to see how many bytes were read.
}

int valid_frame(struct ethframe* frame)
{
    if(frame->type != ETHERNET_TYPE_IP && frame->type != ETHERNET_TYPE_ARP)
        return 0;
    return 1;
}

void __rtl_ext_write(uint8_t* buf, size_t offset, size_t len, ext2_gen_device* dev)
{
    struct ethframe* frame = (struct ethframe*)buf;
    frame->type = htons(frame->type);
    if(!valid_frame(frame))
    {
        kprint("Invalid frame!");
    }
    rtl8139_send_packet(frame, len);
}
// inits a inode and a device
void rtl8139_map_ext2(ext2_inode* inode, ext2_gen_device* dev)
{
    if(inode != NULL_PTR) {
        inode->create_time = cmos_read(SECS);
        inode->type = INODE_TYPE_CHAR_DEV;
        inode->disk_sectors = 0;
    }

    dev->read = __rtl_ext_read;
    dev->write = __rtl_ext_write;
    dev->offset = 0;
    dev->dev_no = 0x8139;
    dev->priv = kalloc(sizeof(ext2_priv_data)); 
}

void rtl8139_handler(struct regs* r){
  uint16_t status = inports(rtl_device.io_base + 0x3e);

  if(status & TOK) {
        
  }
  if (status & ROK) {
    
  }

  outports(rtl_device.io_base + 0x3E, 0x5);
}

int rtl8139_setup()
{
    struct pcidev dev = pci_get_device(RTL8139_VENDOR_ID, RTL8139_DEVICE_ID, -1);
    size_t ret = pci_readt(dev, PCI_BAR0);
    rtl_device.bar_type = ret & 0x1;
    rtl_device.mem_base = ret & (~0xf);
    rtl_device.io_base = ret & (~0x3);
    rtl_device.tx_cur = 0;
    uint32_t pci_command_reg = pci_readt(dev, PCI_COMMAND);
    if(!(pci_command_reg & (1 << 2))) {
        pci_command_reg |= (1 << 2);
        pciwrites(dev.bus, dev.slot, dev.func, PCI_COMMAND, pci_command_reg);
    }
    outportb(rtl_device.io_base + 0x52, 0x0);
    outportb(rtl_device.io_base + 0x37, 0x10);
    while((inportb(rtl_device.io_base + 0x37) & 0x10) != 0) {
        // Do nothibg here...
    }
    rtl_device.rx_buffer = (char*)kalloc(8192 + 16 + 1500, KERN_MEM);
    memset(rtl_device.rx_buffer, 0x0, 8192 + 16 + 1500);
    outportl(rtl_device.io_base + 0x30, (uint32_t)rtl_device.rx_buffer);
    outports(rtl_device.io_base + 0x3C, 0x0005);
    outportl(rtl_device.io_base + 0x44, 0xf | (1 << 7));
    outportb(rtl_device.io_base + 0x37, 0x0C);
    uint32_t irq_num = pci_readt(dev, PCI_INTERRUPT_LINE);
    irq_install_handler(irq_num, rtl8139_handler);
    rtl8139_map_ext2((ext2_inode*)NULL_PTR, netmount);
    ext2_mount(netmount, NULL_PTR);
    return 0;
}

// Get TCP Port not used by any process
uint16_t netallocport()
{
    if(netports == NULL_PTR)
        netports = idr_setup();
    return (uint16_t)idr_alloc_id(netports);
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
#define E1000_REG_RXADDR        0x5400

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

void e1000setmac(e1000info *e, char* mac)
{
    uint32_t low, high;
	memcpy(&low, &mac[0], 4);
	memcpy(&high,&mac[4], 2);
	memset((uint8_t*)&high + 2, 0, 2);
	high |= 0x80000000;
	e1000write_l(e, E1000_REG_RXADDR + 0, low);
	e1000write_l(e, E1000_REG_RXADDR + 4, high);
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

struct etherdev {
    char if_name[32];
	uint8_t mac[6];
	size_t mtu;
	uint32_t ipv4_addr;
	uint32_t ipv4_subnet;
	uint32_t ipv4_gateway;
	uint8_t ipv6_addr[16];
};

typedef struct {
	struct etherdev eth;
	uint32_t pci_device;
	uint16_t deviceid;
	uint32_t* mmio_addr;
	int irq_number;

	int has_eeprom;
	int rx_index;
	int tx_index;
	int link_status;

	struct spinlock tx_lock;

	uint8_t * rx_virt[E1000_NUM_RX_DESC];
	uint8_t * tx_virt[E1000_NUM_TX_DESC];
	volatile struct e1000_rx_desc * rx;
	volatile struct e1000_tx_desc * tx;
	uint32_t* rx_phys;
	uint32_t* tx_phys;

} e1000nic;