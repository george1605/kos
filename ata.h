#pragma once
#include "lib.c"
#include "pci.h"
#include "dma.h"
#include "modules/test.h"
#define ATA_DMA_BOUNDARY 0xffffUL
#define ATA_DMA_MASK 0xffffffffULL
#define ATA_SR_BSY 0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DF 0x20
#define ATA_SR_DSC 0x10
#define ATA_SR_DRQ 0x08
#define ATA_SR_CORR 0x04
#define ATA_SR_IDX 0x02
#define ATA_SR_ERR 0x01

#define ATA_ER_BBK 0x80
#define ATA_ER_UNC 0x40
#define ATA_ER_MC 0x20
#define ATA_ER_IDNF 0x10
#define ATA_ER_MCR 0x08
#define ATA_ER_ABRT 0x04
#define ATA_ER_TK0NF 0x02
#define ATA_ER_AMNF 0x01

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_READ_PIO_EXT 0x24
#define ATA_CMD_READ_DMA 0xC8
#define ATA_CMD_READ_DMA_EXT 0x25
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_WRITE_PIO_EXT 0x34
#define ATA_CMD_WRITE_DMA 0xCA
#define ATA_CMD_WRITE_DMA_EXT 0x35
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET 0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY 0xEC

#define ATAPI_CMD_READ 0xA8
#define ATAPI_CMD_EJECT 0x1B

#define ATA_IDENT_DEVICETYPE 0
#define ATA_IDENT_CYLINDERS 2
#define ATA_IDENT_HEADS 6
#define ATA_IDENT_SECTORS 12
#define ATA_IDENT_SERIAL 20
#define ATA_IDENT_MODEL 54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID 106
#define ATA_IDENT_MAX_LBA 120
#define ATA_IDENT_COMMANDSETS 164
#define ATA_IDENT_MAX_LBA_EXT 200

#define IDE_ATA 0x00
#define IDE_ATAPI 0x01
#define ATA_MASTER 0x00
#define ATA_SLAVE 0x01

#define ATA_REG_DATA 0x00
#define ATA_REG_ERROR 0x01
#define ATA_REG_FEATURES 0x01
#define ATA_REG_SECCOUNT0 0x02
#define ATA_REG_LBA0 0x03
#define ATA_REG_LBA1 0x04
#define ATA_REG_LBA2 0x05
#define ATA_REG_HDDEVSEL 0x06
#define ATA_REG_COMMAND 0x07
#define ATA_REG_STATUS 0x07
#define ATA_REG_SECCOUNT1 0x08
#define ATA_REG_LBA3 0x09
#define ATA_REG_LBA4 0x0A
#define ATA_REG_LBA5 0x0B
#define ATA_REG_CONTROL 0x0C
#define ATA_REG_ALTSTATUS 0x0C
#define ATA_REG_DEVADDRESS 0x0D

#define ATA_PRIMARY 0x00
#define ATA_SECONDARY 0x01
#define ATA_READ 0x00
#define ATA_WRITE 0x01
#define ATA_VENDOR_ID 0x8086
#define ATA_DEVICE_ID 0x7010

#define CONTROL_STOP_INTERRUPT 0x2
#define CONTROL_SOFTWARE_RESET 0x4
#define CONTROL_HIGH_ORDER_BYTE 0x80
#define CONTROL_ZERO 0x00

#define COMMAND_IDENTIFY 0xEC
#define COMMAND_DMA_READ 0xC8
#define ATA_CMD_READ_PIO 0x20

#define STATUS_ERR 0x0
#define STATUS_DRQ 0x8
#define STATUS_SRV 0x10
#define STATUS_DF 0x20
#define STATUS_RDY 0x40
#define STATUS_BSY 0x80

#define BMR_COMMAND_DMA_START 0x1
#define BMR_COMMAND_DMA_STOP 0x0
#define BMR_COMMAND_READ 0x8
#define BMR_STATUS_INT 0x4
#define BMR_STATUS_ERR 0x2

#define SECTOR_SIZE 512
#define MARK_END 0x8000

#define ATA_NOTEXIST 0x12
#define ATA_NOTATA 0x13
#define ATA_POLLERR 0x14

#define MBR_PARTITION_1 0x01BE
#define MBR_PARTITION_2 0x01CE
#define MBR_PARTITION_3 0x01DE
#define MBR_PARTITION_4 0x01EE

#define MBR_BOOTABLE 0x80
#define MBR_REGULAR 0x00
static int mbr_partitions[5] = {MBR_PARTITION_1, MBR_PARTITION_2, MBR_PARTITION_3, MBR_PARTITION_4};
struct prdt
{
    size_t buffer_phys;
    uint16_t transfer_size;
    uint16_t mark_end;
} __attribute__((packed));

size_t ataerr = 0;

struct atadev
{
    // A list of register address
    uint16_t data;
    uint16_t error;
    uint16_t sector_count;

    union
    {
        uint16_t sector_num;
        uint16_t lba_lo;
    };
    union
    {
        uint16_t cylinder_low;
        uint16_t lba_mid;
    };
    union
    {
        uint16_t cylinder_high;
        uint16_t lba_high;
    };
    union
    {
        uint16_t drive;
        uint16_t head;
    };
    union
    {
        uint16_t command;
        uint16_t status;
    };
    union
    {
        uint16_t control;
        uint16_t alt_status;
    };

    int slave;
    size_t bar4;
    size_t BMR_COMMAND;
    size_t BMR_prdt;
    size_t BMR_STATUS;

    struct prdt *prdt;
    uint8_t *prdt_phys;

    uint8_t *mem_buffer;
    uint8_t *mem_buffer_phys;

    char mountpoint[32];
} __attribute__((packed));

struct atadev ata_primary_master = {.slave = 0};
struct atadev ata_primary_slave = {.slave = 1};
struct atadev ata_secondary_master = {.slave = 0};
struct atadev ata_secondary_slave = {.slave = 1};
struct spinlock atapi_lock;

void ata_io_wait(struct atadev *dev)
{
    inportb(dev->alt_status);
    inportb(dev->alt_status);
    inportb(dev->alt_status);
    inportb(dev->alt_status);
}

void ata_reset(struct atadev *dev)
{
    outportb(dev->control, CONTROL_SOFTWARE_RESET);
    ata_io_wait(dev);
    outportb(dev->control, CONTROL_ZERO);
}

void ata_resetd(int num)
{
    if (num == 0)
        ata_reset(&ata_primary_master);
    else if (num == 1)
        ata_reset(&ata_primary_slave);
    else if (num == 2)
        ata_reset(&ata_secondary_master);
    else
        ata_reset(&ata_secondary_slave);
}

void ata_irq_handler(struct regs *r)
{
    struct atadev *dev = r->int_no == 14 ? &ata_primary_master : &ata_secondary_master;
    inportb(dev->alt_status);
    irq_ack(r->int_no);
}

void ata_install()
{
    irq_install_handler(14, ata_irq_handler);
    irq_install_handler(15, ata_irq_handler);
}

void ata_write_sector(struct atadev *dev, size_t lba, char *buf)
{
    memcpy(dev->mem_buffer, buf, SECTOR_SIZE);

    // Reset bus master register's command register
    outportb(dev->control, 0);
    // Set prdt
    outportl(dev->BMR_prdt, (uint32_t)dev->prdt_phys);
    // Select drive
    outportb(dev->drive, 0xe0 | dev->slave << 4 | (lba & 0x0f000000) >> 24);
    // Set sector counts and LBAs
    outportb(dev->sector_count, 1);
    outportb(dev->lba_lo, lba & 0x000000ff);
    outportb(dev->lba_mid, (lba & 0x0000ff00) >> 8);
    outportb(dev->lba_high, (lba & 0x00ff0000) >> 16);

    // Write the WRITE_DMA to the command register (0xCA)
    outportb(dev->command, 0xCA);
    outportb(dev->BMR_COMMAND, 0x1);

    while (1)
    {
        int status = inportb(dev->BMR_STATUS);
        int dstatus = inportb(dev->status);
        if (!(status & 0x04))
        {
            continue;
        }
        if (!(dstatus & 0x80))
        {
            break;
        }
    }
}

char *ata_kread_sector(char* buf, struct atadev *dev, size_t lba)
{
    outportb(dev->BMR_COMMAND, 0);
    // Set prdt
    outportl(dev->BMR_prdt, (size_t)dev->prdt_phys);
    // Select drive
    outportb(dev->drive, 0xe0 | dev->slave << 4 | (lba & 0x0f000000) >> 24);
    // Set sector counts and LBAs
    outportb(dev->sector_count, 1);
    outportb(dev->lba_lo, lba & 0x000000ff);
    outportb(dev->lba_mid, (lba & 0x0000ff00) >> 8);
    outportb(dev->lba_high, (lba & 0x00ff0000) >> 16);

    outportb(dev->command, 0xC8);
    outportb(dev->BMR_COMMAND, 0x8 | 0x1);

    while (1)
    {
        int status = inportb(dev->BMR_STATUS);
        int dstatus = inportb(dev->status);
        if (!(status & 0x04))
        {
            continue;
        }
        if (!(dstatus & 0x80))
        {
            break;
        }
    }

    memcpy(buf, dev->mem_buffer, SECTOR_SIZE);
    return buf;
}

// making code more DRY
char* ata_read_sector(struct atadev *dev, size_t lba)
{
    char* s = (char*)kalloc(SECTOR_SIZE, KERN_MEM);
    ata_kread_sector(s, dev, lba);
    return s;
}

void ata_device_init(struct atadev *dev, int primary)
{

    // Setup DMA
    // Prdt must not cross 64kb boundary / contiguous in physical memory. So simply allocate a 4kb aligned page satisfy both conditions
    dev->prdt = (void *)kalloc(sizeof(struct prdt), KERN_MEM);
    memset(dev->prdt, 0, sizeof(struct prdt));
    dev->prdt_phys = dev->prdt;
    dev->mem_buffer = (void *)kalloc(4096, KERN_MEM);
    memset(dev->mem_buffer, 0, 4096);

    dev->prdt[0].buffer_phys = (uint32_t)dev->mem_buffer;
    dev->prdt[0].transfer_size = SECTOR_SIZE;
    dev->prdt[0].mark_end = MARK_END;

    // Setup register address
    uint16_t base_addr = primary ? (0x1F0) : (0x170);
    uint16_t alt_status = primary ? (0x3F6) : (0x376);

    dev->data = base_addr;
    dev->error = base_addr + 1;
    dev->sector_count = base_addr + 2;
    dev->lba_lo = base_addr + 3;
    dev->lba_mid = base_addr + 4;
    dev->lba_high = base_addr + 5;
    dev->drive = base_addr + 6;
    dev->command = base_addr + 7;
    dev->alt_status = alt_status;

    dev->bar4 = pci_readt(ata_device, PCI_BAR4);
    if (dev->bar4 & 0x1)
    {
        dev->bar4 = dev->bar4 & 0xfffffffc;
    }
    dev->BMR_COMMAND = dev->bar4;
    dev->BMR_STATUS = dev->bar4 + 2;
    dev->BMR_prdt = dev->bar4 + 4;

    memset(dev->mountpoint, 0, 32);
    strcpy(dev->mountpoint, "home/dev/hd");
}

void ata_device_detect(struct atadev *dev, int primary)
{

    ata_device_init(dev, primary);

    ata_reset(dev);
    ata_io_wait(dev);
    // Select drive, send 0xA0 to master device, 0xB0 to slave device
    outportb(dev->drive, (0xA + dev->slave) << 4);
    outportb(dev->sector_count, 0);
    outportb(dev->lba_lo, 0);
    outportb(dev->lba_mid, 0);
    outportb(dev->lba_high, 0);

    outportb(dev->command, COMMAND_IDENTIFY);
    if (!inportb(dev->status))
    {
        ataerr = ATA_NOTEXIST;
        return;
    }

    uint8_t lba_lo = inportb(dev->lba_lo);
    uint8_t lba_hi = inportb(dev->lba_high);
    if (lba_lo != 0 || lba_hi != 0)
    {
        ataerr = ATA_NOTATA;
        return;
    }
    uint8_t drq = 0, err = 0;
    while (!drq && !err)
    {
        drq = inportb(dev->status) & STATUS_DRQ;
        err = inportb(dev->status) & STATUS_ERR;
    }
    if (err)
    {
        ataerr = ATA_POLLERR;
        return;
    }

    for (int i = 0; i < 256; i++)
        inports(dev->data);

    size_t pci_command_reg = pciget(ata_device, 0x04);
    if (!(pci_command_reg & (1 << 2)))
    {
        pci_command_reg |= (1 << 2);
        pciset(ata_device, 0x04, pci_command_reg);
    }
}

void ata_init()
{
    ata_device.vendor = ATA_VENDOR_ID;
    ata_device.device = ATA_DEVICE_ID;

    ata_install(); // installs the 14 and 15 IRQs

    ata_device_detect(&ata_primary_master, 1);
    ata_device_detect(&ata_primary_slave, 1);
    ata_device_detect(&ata_secondary_master, 0);
    ata_device_detect(&ata_secondary_slave, 0);
}

typedef struct {
  uint8_t  status;
  uint8_t  chs_first_sector[3];
  uint8_t  type;
  uint8_t  chs_last_sector[3];
  uint32_t lba_first_sector;
  uint32_t sector_count;
} mbr_partition;

typedef struct {
  uint16_t drive_no;
  uint32_t start_lba;
  uint32_t end_lba;
  uint16_t flags;
} fs_partition;

void ata_open_disk(uint32_t disk, uint8_t partition, mbr_partition *out) {
  disk = disk; // future plans
  uint8_t *raw = (uint8_t*)kalloc(SECTOR_SIZE, KERN_MEM);
  struct atadev* tdev = (disk == 0)? &ata_primary_master : &ata_primary_slave;
  ata_kread_sector((char*)raw, tdev, 0x0);
  *out = *(mbr_partition*)(&raw[mbr_partitions[partition]]);
  free(raw);
}

inline uint8_t ata_is_ext2(mbr_partition *out) 
{
    return out->type == 0x83;
}

size_t ata_partition_size(fs_partition* fspart) 
{
    return (fspart->end_lba - fspart->start_lba) * SECTOR_SIZE;
}

void ata_partition_split(fs_partition in, size_t offset, fs_partition* out[2])
{
    out[0]->start_lba = in.start_lba;
    out[0]->end_lba = in.start_lba + offset;
    out[1]->start_lba = in.start_lba + offset + 1;
    out[1]->end_lba = in.end_lba;
}