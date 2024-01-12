#pragma once
#include "../ata.h"
#include "../pci.h"
#include "../smem.h"
#include "../buf.h"
#define EXT2_SIGNATURE 0xEF53

typedef struct {
	uint32_t inodes;
	uint32_t blocks;
	uint32_t reserved_for_root;
	uint32_t unallocatedblocks;
	uint32_t unallocatedinodes;
	uint32_t superblock_id;
	uint32_t blocksize_hint; // shift by 1024 to the left
	uint32_t fragmentsize_hint; // shift by 1024 to left
	uint32_t blocks_in_blockgroup;
	uint32_t frags_in_blockgroup;
	uint32_t inodes_in_blockgroup;
	uint32_t last_mount;
	uint32_t last_write;
	uint16_t mounts_since_last_check;
	uint16_t max_mounts_since_last_check;
	uint16_t ext2_sig; // 0xEF53
	uint16_t state;
	uint16_t op_on_err;
	uint16_t minor_version;
	uint32_t last_check;
	uint32_t max_time_in_checks;
	uint32_t os_id;
	uint32_t major_version;
	uint16_t uuid;
	uint16_t gid;
	uint8_t unused[940];
} __attribute__((packed)) superblock_t;

typedef struct {
	uint32_t block_of_block_usage_bitmap;
	uint32_t block_of_inode_usage_bitmap;
	uint32_t block_of_inode_table;
	uint16_t num_of_unalloc_block;
	uint16_t num_of_unalloc_inode;
	uint16_t num_of_dirs;
	uint8_t unused[14];
} __attribute__((packed)) block_group_desc_t;

uint8_t ext2_errno;
#define EXT2_NO_READ 1
#define EXT2_NO_WRITE 2
#define EXT2_OVER_DPB 4
#define EXT2_BAD_SIGN 8
#define EXT2_SIZE_OF_SINGLY (priv->blocksize * priv->blocksize / 4)

#define INODE_TYPE_FIFO 0x1000
#define INODE_TYPE_CHAR_DEV 0x2000
#define INODE_TYPE_DIRECTORY 0x4000
#define INODE_TYPE_BLOCK_DEV 0x6000
#define INODE_TYPE_FILE 0x8000
#define INODE_TYPE_SYMLINK 0xA000
#define INODE_TYPE_SOCKET 0xC000

#define TRANSACTION_WRITE 1
#define TRANSACTION_READ 2
#define TRANSACTION_MKDIR 3
#define TRANSACTION_TOUCH 4

typedef struct {
	uint16_t type;
	uint16_t uid;
	uint32_t size;
	uint32_t last_access;
	uint32_t create_time;
	uint32_t last_modif;
	uint32_t delete_time;
	uint16_t gid;
	uint16_t hardlinks;
	uint32_t disk_sectors;
	uint32_t flags;
	uint32_t ossv1;
	uint32_t dbp[12];
	uint32_t singly_block;
	uint32_t doubly_block;
	uint32_t triply_block;
	uint32_t gen_no;
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t fragment_block;
	uint8_t ossv2[12];
} __attribute__((packed)) ext2_inode;

typedef struct __ext2_dir_entry {
	uint32_t inode;
	uint16_t size;
	uint8_t namelength;
	uint8_t reserved;
	/* name here */
} __attribute__((packed)) ext2_dir;

typedef struct __ext2_priv_data {
	superblock_t sb;
	uint32_t first_bgd;
	uint32_t number_of_bgs;
	uint32_t blocksize;
	uint32_t sectors_per_block;
	uint32_t inodes_per_block;
} __attribute__((packed)) ext2_priv_data;

struct __fs_t;

typedef struct __ext2_gen_device {
    char* name;
    int dev_no;
	void* priv; // private data
    size_t offset; // keep the offset, can be changed using ext2_dev_seek()
	struct __fs_t* fs;
    void(*read)(uint8_t*, size_t offset, size_t len, struct __ext2_gen_device*);
    void(*write)(uint8_t*, size_t offset, size_t len, struct __ext2_gen_device*);
} ext2_gen_device;

typedef struct {
	ext2_inode* inode;
	void* buffer; // data to commit to inode
	int type;
	size_t size;
} ext2_transaction;

typedef struct __fs_t {
	char *name;
	uint8_t (*probe)(ext2_gen_device *);
	uint8_t (*read)(char *, char *, ext2_gen_device *, void *);
	uint8_t (*read_dir)(char *, char *, ext2_gen_device *, void *);
	uint8_t (*touch)(char *fn, ext2_gen_device*, void *);
	uint8_t (*writefile)(char *fn, char *buf, uint32_t len, ext2_gen_device*, void *);
	uint8_t (*exist)(char *filename, ext2_gen_device*, void *);
	uint8_t (*mount)(ext2_gen_device*, void *);
	uint8_t *priv_data;
} filesystem;

static ext2_inode *inode = 0;
static uint8_t *root_buf = 0;
static uint8_t *block_buf = 0;

static void __ext2_ata_read(uint8_t* buffer, size_t offset, size_t len, struct __ext2_gen_device* device)
{
    size_t i = offset / 512, sz = 0;
    struct atadev* ata = (device->dev_no == 0) ? &ata_primary_master : &ata_primary_slave;
    while(sz < len) {
        ata_kread_sector((char*)(buffer + sz), ata, i);
		i++;
        sz += 512;
    }
    device->offset += len / 512;
}

static void __ext2_ata_write(uint8_t* buffer, size_t offset, size_t len, struct __ext2_gen_device* device)
{
    size_t i = offset / 512, sz = 0;
    struct atadev* ata = (device->dev_no == 0) ? &ata_primary_master : &ata_primary_slave;
    while(sz < len) {
        ata_write_sector(ata, i, (char*)(buffer + sz));
		i++;
        sz += 512;
    }
    device->offset += len / 512;
}

ext2_gen_device* ext2_from_ata(int ata_id)
{
    ext2_gen_device* dev = (ext2_gen_device*)kalloc(sizeof(ext2_gen_device), KERN_MEM);
    dev->dev_no = ata_id;
    dev->offset = 0; // read from the bootloader in theory
    dev->read = __ext2_ata_read;
    dev->write = __ext2_ata_write;
    return dev;
}

void* ext2_from_buf(struct buf* bufs, ext2_priv_data* priv)
{
	struct buf* b = &bufs[0];
	int sz = 0;
	void* mem = kalloc(priv->blocksize * priv->sb.blocks, KERN_MEM), *p = mem;
	while(b != NULL_PTR && sz < priv->blocksize)
	{
		memcpy(mem, b->data, 512);
		mem += 512, sz += 512;
		b = b->next;
	}
	return p;
}

void ext2_read_block(uint8_t *buf, uint32_t block, ext2_gen_device *dev, ext2_priv_data *priv)
{
	uint32_t sectors_per_block = priv->sectors_per_block;
	if(!sectors_per_block) sectors_per_block = 1;
	dev->read(buf, block*sectors_per_block, sectors_per_block, dev);
}

void ext2_write_block(uint8_t *buf, uint32_t block, ext2_gen_device *dev, ext2_priv_data *priv)
{
	uint32_t sectors_per_block = priv->sectors_per_block;
	if(!sectors_per_block) sectors_per_block = 1;
	dev->write(buf, block*sectors_per_block, sectors_per_block, dev);
}

void ext2_read_inode(ext2_inode *inode_buf, uint32_t inode, ext2_gen_device *dev, ext2_priv_data *priv)
{
	uint32_t bg = (inode - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;
	if(!block_buf)
         block_buf = (uint8_t *)kalloc(priv->blocksize, KERN_MEM);
	ext2_read_block(block_buf, priv->first_bgd, dev, priv);
	block_group_desc_t *bgd = (block_group_desc_t*)block_buf;
	for(i = 0; i < bg; i++)
		bgd++;

	uint32_t index = (inode - 1) % priv->sb.inodes_in_blockgroup;
	uint32_t block = (index * sizeof(ext2_inode))/ priv->blocksize;
	ext2_read_block(block_buf, bgd->block_of_inode_table + block, dev, priv);
	ext2_inode* _inode = (ext2_inode *)block_buf;
	index = index % priv->inodes_per_block;
	for(i = 0; i < index; i++)
		_inode++;
	memcpy(inode_buf, _inode, sizeof(ext2_inode));
}

void ext2_write_inode(ext2_inode *inode_buf, uint32_t ii, ext2_gen_device *dev, ext2_priv_data *priv)
{
	uint32_t bg = (ii - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;
	if(!block_buf) block_buf = (uint8_t *)kalloc(priv->blocksize, KERN_MEM);
	ext2_read_block(block_buf, priv->first_bgd, dev, priv);
	block_group_desc_t *bgd = (block_group_desc_t*)block_buf;

	for(i = 0; i < bg; i++)
		bgd++;
	uint32_t index = (ii - 1) % priv->sb.inodes_in_blockgroup;
	uint32_t block = (index * sizeof(ext2_inode))/ priv->blocksize;
	uint32_t final = bgd->block_of_inode_table + block;
	ext2_read_block(block_buf, final, dev, priv);
	ext2_inode* _inode = (ext2_inode *)block_buf;
	index = index % priv->inodes_per_block;
	for(i = 0; i < index; i++)
		_inode++;
	memcpy(_inode, inode_buf, sizeof(ext2_inode));
	ext2_write_block(block_buf, final, dev, priv);
}

uint32_t ext2_get_inode_block(uint32_t inode, uint32_t *b, uint32_t *ioff, ext2_gen_device *dev, ext2_priv_data *priv)
{
	uint32_t bg = (inode - 1) / priv->sb.inodes_in_blockgroup;
	uint32_t i = 0;
	if(!block_buf) block_buf = (uint8_t *)kalloc(priv->blocksize, KERN_MEM);
	ext2_read_block(block_buf, priv->first_bgd, dev, priv);
	block_group_desc_t *bgd = (block_group_desc_t*)block_buf;
	for(i = 0; i < bg; i++)
		bgd++;
	uint32_t index = (inode - 1) % priv->sb.inodes_in_blockgroup;
	uint32_t block = (index * sizeof(ext2_inode))/ priv->blocksize;
	index = index % priv->inodes_per_block;
	*b = block + bgd->block_of_inode_table;
	*ioff = index;
	return 1;
}

uint32_t ext2_read_directory(char *filename, ext2_dir *dir, ext2_gen_device *dev, ext2_priv_data *priv)
{
	while(dir->inode != 0) {
		char *name = (char*)kalloc(dir->namelength + 1, KERN_MEM);
		memcpy(name, &dir->reserved+1, dir->namelength);
		name[dir->namelength] = 0;
		if(filename && strcmp(filename, name) == 0)
		{
			ext2_read_inode(inode, dir->inode, dev, priv);
			free(name);
			return dir->inode;
		}
		dir = (ext2_dir*)((uint32_t)dir + dir->size);
		free(name);
	}
	return 0;
}

uint8_t ext2_read_root_directory(char *filename, ext2_gen_device *dev, ext2_priv_data *priv)
{
	if(!inode) inode = (ext2_inode*)kalloc(sizeof(ext2_inode), KERN_MEM);
	if(!root_buf) root_buf = (uint8_t*)kalloc(priv->blocksize, KERN_MEM);
	ext2_read_inode(inode, 2, dev, priv);
	if((inode->type & 0xF000) != INODE_TYPE_DIRECTORY)
	{
		return 0;
	}
	for(int i = 0;i < 12; i++)
	{
		uint32_t b = inode->dbp[i];
		if(b == 0) break;
		ext2_read_block(root_buf, b, dev, priv);
		/* Now loop through the entries of the directory */
		if(ext2_read_directory(filename, (ext2_dir*)root_buf, dev, priv)) return 1;
	}
	if(filename && (uint32_t)filename != 1) return 0;
	return 1;
}

uint8_t ext2_find_file_inode(char *ff, ext2_inode *inode_buf, ext2_gen_device *dev, ext2_priv_data *priv)
{
	char *filename = (char*)kalloc(strlen(ff) + 1, KERN_MEM);
	memcpy(filename, ff, strlen(ff) +1);
	size_t n = strchr(filename, '/');
	filename++; // skip the first crap
	uint32_t retnode = 0;
	if(n > 1)
	{ 
		ext2_read_inode(inode, 2, dev, priv); // reads the root inode
		n--;
		while(n--)
		{
			for(int i = 0; i < 12; i++)
			{
				uint32_t b = inode->dbp[i];
				if(!b) break;
				ext2_read_block(root_buf, b, dev, priv);
				uint32_t rc = ext2_read_directory(filename, (ext2_dir *)root_buf, dev, priv);
				if(!rc)
				{
					if(strcmp(filename, "") == 0)
					{
						free(filename);
						return strcmp(ff, "/")?retnode:1;
					}
					free(filename);
					return 0;
				} else {
					 retnode = rc;
					 goto fix;
				}
			}
			fix:;
			uint32_t s = strlen(filename);
			filename += s + 1;
		}
		memcpy(inode_buf, inode, sizeof(ext2_inode));
	} else {
		ext2_read_root_directory(filename, dev, priv);
		memcpy(inode_buf, inode, sizeof(ext2_inode));
	}
	free(filename);
	return retnode;
}

void ext2_list_directory(char *dd, char *buffer, ext2_gen_device *dev, ext2_priv_data *priv)
{
	char *dir = dd;
	int rc = ext2_find_file_inode(dir, (ext2_inode*)buffer, dev, priv);
	if(!rc) return;
	for(int i = 0;i < 12; i++)
	{
		uint32_t b = inode->dbp[i];
		if(!b) break;
		ext2_read_block(root_buf, b, dev, priv);
		ext2_read_directory(0, (ext2_dir *)root_buf, dev, priv);
	}
}

uint8_t ext2_read_singly_linked(uint32_t blockid, uint8_t *buf, ext2_gen_device *dev, ext2_priv_data *priv)
{
	uint32_t blockadded = 0;
	uint32_t maxblocks = ((priv->blocksize) / (sizeof(uint32_t)));
	 ext2_read_block(root_buf, blockid, dev, priv);
	 uint32_t *block = (uint32_t *)root_buf;
	 for(int i =0;i < maxblocks; i++)
	 {
	 	if(block[i] == 0) break;
	 	ext2_read_block(buf + i * priv->blocksize, block[i], dev, priv);
	 }
	 return 1;
}

uint8_t ext2_read_doubly_linked(uint32_t blockid, uint8_t *buf, ext2_gen_device *dev, ext2_priv_data *priv)
{
	uint32_t blockadded = 0;
	uint32_t maxblocks = ((priv->blocksize) / (sizeof(uint32_t)));
	ext2_read_block(block_buf, blockid, dev, priv);
	uint32_t *block = (uint32_t *)block_buf;
	uint32_t s = EXT2_SIZE_OF_SINGLY;
	for(int i = 0;i < maxblocks; i++)
	{
	 	if(block[i] == 0) break;
	 	ext2_read_singly_linked(block[i], buf + i * s , dev, priv);
	}
	return 1;
}

static ext2_inode *minode = 0;
uint8_t ext2_read_file(char *fn, char *buffer, ext2_gen_device *dev, ext2_priv_data *priv)
{
	if(!minode) minode = (ext2_inode*)kalloc(sizeof(ext2_inode), KERN_MEM);
	char *filename = fn;
	if(!ext2_find_file_inode(filename, minode, dev, priv))
	{
		
		return 0;
	}
	for(int i = 0; i < 12; i++)
	{
		uint32_t b = minode->dbp[i];
		if(b == 0) { return 1; }
		if(b > priv->sb.blocks) {} // outside range
		ext2_read_block(root_buf, b, dev, priv);
		memcpy(buffer + i*(priv->blocksize), root_buf, priv->blocksize);
	}
	if(minode->singly_block) {
		ext2_read_singly_linked(minode->singly_block, (uint8_t*)(buffer + 12*(priv->blocksize)), dev, priv);
	}
	if(minode->doubly_block) {
		uint32_t s = EXT2_SIZE_OF_SINGLY + 12*priv->blocksize;
		ext2_read_doubly_linked(minode->doubly_block, (uint8_t*)(buffer + s), dev, priv);
	}
	return 1;
}

void ext2_find_new_inode_id(uint32_t *id, ext2_gen_device *dev, ext2_priv_data *priv)
{
	ext2_read_block(root_buf, priv->first_bgd, dev, priv);
	block_group_desc_t *bg = (block_group_desc_t *)root_buf;
	for(int i = 0; i < priv->number_of_bgs; i++)
	{
		if(bg->num_of_unalloc_inode)
		{
			 *id = ((i + 1) * priv->sb.inodes_in_blockgroup) - bg->num_of_unalloc_inode + 1;
			 bg->num_of_unalloc_inode --;
			 ext2_write_block(root_buf, priv->first_bgd + i, dev, priv);
			 ext2_read_block(root_buf, priv->sb.superblock_id, dev, priv);
			 superblock_t *sb = (superblock_t *)root_buf;
			 sb->unallocatedinodes --;
			 ext2_write_block(root_buf, priv->sb.superblock_id, dev, priv);
			 return;
		}
		bg++;
	}
}

void ext2_alloc_block(uint32_t *out, ext2_gen_device *dev, ext2_priv_data *priv)
{
	 ext2_read_block(root_buf, priv->first_bgd, dev, priv);
	 block_group_desc_t *bg = (block_group_desc_t *)root_buf;
	 for(int i = 0; i < priv->number_of_bgs; i++)
	 {
	 	if(bg->num_of_unalloc_block)
	 	{
	 		*out = priv->sb.blocks - bg->num_of_unalloc_block + 1;
	 		bg->num_of_unalloc_block --;
	 		ext2_write_block(root_buf, priv->first_bgd + i, dev, priv);
	 		ext2_read_block(root_buf, priv->sb.superblock_id, dev, priv);
			superblock_t *sb = (superblock_t *)root_buf;
			sb->unallocatedblocks --;
			ext2_write_block(root_buf, priv->sb.superblock_id, dev, priv);
			return;
	 	}
	 	bg++;
	 }
}

void ext2_alloc_range(int num, uint32_t* start, uint32_t* end, ext2_gen_device* dev, ext2_priv_data* data)
{
	ext2_alloc_block(start, dev, data);
	uint32_t temp;
	for(int i = 1;i < num;i++)
	{
		ext2_alloc_block(&temp, dev, data);
	}
	ext2_alloc_block(end, dev, data);
}

uint8_t ext2_touch(char *file, ext2_gen_device *dev, ext2_priv_data *priv)
{
	if(!dev->write)
		return 0;
	char *fil = (char *)kalloc(strlen(file) + 1, KERN_MEM);
	memcpy(fil, file, strlen(file) + 1);
	ext2_inode* fi = (ext2_inode*)kalloc(sizeof(ext2_inode), KERN_MEM);
	fi->hardlinks = 1;
	fi->size = 0;
	fi->type = INODE_TYPE_FILE;
	fi->disk_sectors = 2;
	size_t n = strchr(fil, '/');
	n--;
	while(n)
	{
		fil += strlen(fil) + 1;
		n--;
	}
	ext2_dir *entry = (ext2_dir *)kalloc(sizeof(ext2_dir) + strlen(fil) + 1, KERN_MEM);
	entry->size = sizeof(ext2_dir) + strlen(fil) + 1;
	entry->namelength = strlen(fil) + 1;
	entry->reserved = 0;
	memcpy(&entry->reserved + 1, fil, strlen(fil) + 1);

	uint32_t id = 0;
	ext2_find_new_inode_id(&id, dev, priv);
	entry->inode = id;
	uint32_t block = 0; /* The block where this inode should be written */
	uint32_t ioff = 0; /* Offset into the block function to sizeof(inode_t) */
	ext2_get_inode_block(id, &block, &ioff, dev, priv);
	//kprintf("This inode is located on block %d with ioff %d\n", block, ioff);
	/* First, read the block in */
	ext2_read_block(root_buf, block, dev, priv);
	ext2_inode *winode = (ext2_inode*)root_buf;
	for(int i = 0;i < ioff; i++)
		winode++;
	memcpy(winode, fi, sizeof(ext2_inode));
	ext2_write_block(root_buf, block, dev, priv);
	char *f = (char *)kalloc(strlen(file) + 1, KERN_MEM);
	memcpy(f, file, strlen(file) + 1);
	str_backspace(f, '/');

	//kprintf("LF: %s\n", f);
	if(!inode) inode = (ext2_inode*)kalloc(sizeof(ext2_inode), KERN_MEM);
	if(!block_buf) block_buf = (uint8_t *)kalloc(priv->blocksize, KERN_MEM);
	uint32_t t = ext2_find_file_inode(f, inode, dev, priv);
	t++;
	//kprintf("Parent is inode %d\n", t);
	uint8_t found = 0;
	for(int i = 0; i < 12; i++)
	{
		/* Loop through the dpb to find an empty spot */
		if(inode->dbp[i] == 0)
		{
			uint32_t theblock = 0;
			ext2_alloc_block(&theblock, dev, priv);
			inode->dbp[i] = theblock;
			ext2_write_inode(inode, t, dev, priv);
 		}
		ext2_read_block(block_buf, inode->dbp[i], dev, priv);
		ext2_dir *d = (ext2_dir *)block_buf;
		uint32_t passed = 0;
		while(d->inode != 0) {
			if(d->size == 0) break;
			uint32_t truesize = d->namelength + 8;
			truesize += 4 - truesize % 4;
			uint32_t origsize = d->size;
			if(truesize != d->size)
			{
				d->size = truesize;
				passed += d->size;
				d = (ext2_dir *)((uint32_t)d + d->size);
				entry->size = priv->blocksize - passed;
				break;
			}
			passed += d->size;
			d = (ext2_dir *)((uint32_t)d + d->size);
		}
		if(passed >= priv->blocksize)
		{
			continue;
		}
	dir_write:
		memcpy(d, entry, entry->size);
		ext2_write_block(block_buf, inode->dbp[i], dev, priv);
		return 1;
	}
	return 0;
}

uint8_t ext2_exist(char *file, ext2_gen_device *dev, ext2_priv_data *priv)
{
	return ext2_read_file(file, 0, dev, priv);
}

void ext2_free_range(size_t first, size_t end, ext2_gen_device* dev, ext2_priv_data* priv)
{

}

void ext2_removefile(char* fn, ext2_gen_device* dev, ext2_priv_data* priv)
{
	uint32_t inode_id = ext2_find_file_inode(fn, inode, dev, priv);
	ext2_read_inode(inode, inode_id, dev, priv);
	if(inode->dbp[1] - inode->dbp[0] >= 12)
	{
		ext2_free_range(inode->dbp[0], inode->dbp[1], dev, priv);
	} else {
		for(int i = 0;i < 12;i++)
			inode->dbp[i] = 0; // sets it to 0 which means it is unused
	}
}

uint8_t ext2_writefile(char *fn, char *buf, uint32_t len, ext2_gen_device *dev, ext2_priv_data *priv)
{
	uint32_t inode_id = ext2_find_file_inode(fn, inode, dev, priv);
	inode_id ++;
	if(inode_id == 1) return 0;
	if(!inode) inode = (ext2_inode*)kalloc(sizeof(ext2_inode), KERN_MEM);
	ext2_read_inode(inode, inode_id, dev, priv);
	if(inode->size == 0)
	{
		uint32_t blocks_to_alloc = len / priv->blocksize;
		blocks_to_alloc ++;
		if(blocks_to_alloc > 12)
		{
			ext2_alloc_range(blocks_to_alloc, &inode->dbp[0], &inode->dbp[1], dev, priv); // added this, pretty good, right?
		} else {
			for(int i = 0; i < blocks_to_alloc; i++)
			{
				uint32_t bid = 0;
				ext2_alloc_block(&bid, dev, priv);
				inode->dbp[i] = bid;
			}
		}
		inode->size += len; 
		ext2_write_inode(inode, inode_id - 1, dev, priv);
		for(int i = 0; i < blocks_to_alloc; i++)
		{
			ext2_read_block(root_buf, inode->dbp[i], dev, priv);
			if(i + 1 < blocks_to_alloc) { // If not last block
				memcpy(root_buf, buf + i*priv->blocksize, priv->blocksize);
			} else {// If last block
				memcpy(root_buf, buf + i*priv->blocksize, len);
			}
			ext2_write_block(root_buf, inode->dbp[i], dev, priv);
		}
		return 1;
	}
	uint32_t last_data_block = inode->size / priv->blocksize;
	uint32_t last_data_offset = (inode->size) % priv->blocksize;
	ext2_read_block(root_buf, last_data_block, dev, priv);
	if(len < priv->blocksize - last_data_offset)
	{
		memcpy(root_buf + last_data_offset, buf, len);
		ext2_write_block(root_buf, last_data_block, dev, priv);
		return 1;
	}
 	return 0;
}

uint8_t ext2_probe(ext2_gen_device *dev)
{
	if(!dev->read)
	{
		ext2_errno = EXT2_NO_READ;
		return 0;
	}
	uint8_t *buf = (uint8_t *)kalloc(1024, KERN_MEM);
	dev->read(buf, 2, 2, dev);
	superblock_t *sb = (superblock_t *)buf;
	if(sb->ext2_sig != EXT2_SIGNATURE)
	{
		ext2_errno = EXT2_BAD_SIGN;
		free(buf);
		return 0;
	}
	filesystem *fs = (filesystem*)kalloc(sizeof(filesystem), KERN_MEM);
	ext2_priv_data *priv = (ext2_priv_data *)kalloc(sizeof(ext2_priv_data), KERN_MEM);
	memcpy(&priv->sb, sb, sizeof(superblock_t));

	uint32_t blocksize = 1024 << sb->blocksize_hint;
	priv->blocksize = blocksize;
	priv->inodes_per_block = blocksize / sizeof(ext2_gen_device);
	priv->sectors_per_block = blocksize / 512;
	/* Calculate the number of block groups */
	uint32_t number_of_bgs0 = sb->blocks / sb->blocks_in_blockgroup;
	if(!number_of_bgs0) number_of_bgs0 = 1;
	printf("There are %d block group(s).\n", number_of_bgs0);
	priv->number_of_bgs = number_of_bgs0;

	uint32_t block_bgdt = sb->superblock_id + (sizeof(superblock_t) / blocksize);
	priv->first_bgd = block_bgdt;
	fs->name = "EXT2";
	fs->probe = (uint8_t(*)(ext2_gen_device*)) ext2_probe;
	fs->mount = (uint8_t(*)(ext2_gen_device*, void *)) ext2_mount;
	fs->read = (uint8_t(*)(char *, char *, ext2_gen_device*, void *)) ext2_read_file;
	fs->exist = (uint8_t(*)(char *, ext2_gen_device*, void *)) ext2_exist;
	fs->read_dir = (uint8_t(*)(char * , char *, ext2_gen_device*, void *)) ext2_list_directory;
	fs->touch = (uint8_t(*)(char *, ext2_gen_device*, void *)) ext2_touch;
	fs->writefile = (uint8_t(*)(char *, char *m, uint32_t, ext2_gen_device*, void *)) ext2_writefile;
	fs->priv_data = (uint8_t*)priv;
	dev->fs = fs;
	free(buf);
	//free(buffer);
	return 1;
}

#define MAX_FS_BLOCKS (uint32_t)(0xffffffff)

void ext2_setup_drive(ext2_gen_device* dev, superblock_t* sb)
{
	sb->unallocatedblocks = sb->unallocatedinodes = MAX_FS_BLOCKS;
	sb->ext2_sig = EXT2_SIGNATURE;
	sb->last_mount = 0;
	sb->last_check = sb->last_write = cmos_read(SECS); // get the unix time
	sb->blocksize_hint = 4; // or whatever, need to change it up (the unix time also)
	dev->write((uint8_t*)sb, 2, 2, dev);
}

uint8_t ext2_mount(ext2_gen_device *dev, void *privd)
{
	ext2_priv_data *priv = (ext2_priv_data*)privd;
	if(ext2_read_root_directory((char *)1, dev, priv))
		return 1;
	return 0;
}