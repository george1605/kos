#include "../fs.h"
#include "../drivers/cdrom.h"
#define ISO_RES_START 0x0
#define ISO_RES_END 0x7

typedef struct { uint8_t le[2]; }          iso_luint16;
typedef struct { uint8_t be[2]; }          iso_buint16;
typedef struct { uint8_t le[2], be[2]; }   iso_duint16;
typedef struct { uint8_t le[4]; }          iso_luint32;
typedef struct { uint8_t be[4]; }          iso_buint32;
typedef struct { uint8_t le[4], be[4]; }   iso_duint32;

struct iso_header
{
  char typecode;
  char ident[5]; // 'CD001'
  char version;
  char unused; // 0x0
  char s_ident[32];
  char v_ident[32];
  char unused2[8];
} __attribute__((packed));

struct iso_voldesc
{
  uint8_t type;
  uint8_t magic[5];
  uint8_t version;
} __attribute__((packed));

struct iso_date2
{
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t offset;
} __attribute__((packed));

typedef struct {
    char d[17];
} iso_desctime;

/* File time format */
typedef struct {
    char d[7];
} iso_filetime;

/* Directory entry */
typedef struct {
    uint8_t         length;
    uint8_t         xattr_length;
    iso_duint32     sector;
    iso_duint32     size;
    iso_filetime    time;
    uint8_t         flags;
    uint8_t         unit_size;
    uint8_t         gap_size;
    iso_duint16     vol_seq_number;
    uint8_t         name_len;
    char            name[/*name_len*/];
} iso_dirent;

/* Volume descriptor header */
typedef struct {
    uint8_t type;
    char    magic[5];
    uint8_t version;
} iso_vdesc_header;

/* Primary volume descriptor */
typedef struct {
    iso_vdesc_header  hdr;
    char              pad0[1];
    char              system_id[32];
    char              volume_id[32];
    char              pad1[8];
    iso_duint32       volume_space_size;
    char              pad2[32];
    iso_duint16       volume_set_size;
    iso_duint16       volume_seq_number;
    iso_duint16       logical_block_size;
    iso_duint32       path_table_size;
    iso_luint32       path_table_le;
    iso_luint32       path_table_opt_le;
    iso_buint32       path_table_be;
    iso_buint32       path_table_opt_be;
    union {
        iso_dirent    root_dir_ent;
        char            pad3[34];
    };
    char                volume_set_id[128];
    char                data_preparer_id[128];
    char                app_id[128];
    char                copyright_file[38];
    char                abstract_file[36];
    char                bibliography_file[37];
    iso_desctime        volume_created,
                        volume_modified,
                        volume_expires,
                        volume_effective;
    uint8_t             file_structure_version;
    char                pad4[1];
    char                app_reserved[512];
    char                reserved[653];
} iso_vdesc_primary;

typedef union {
    iso_vdesc_header  hdr;
    char                _bits[2048];
} iso_vdesc;

typedef struct iso_fs {
    union {
        iso_dirent    root_dir_ent;
        char          root_dir_pad[34];
    };
    bool (*read_sector)(struct iso_fs *fs, void *buf, uint32_t sector);
} iso_fs;

typedef struct {
    char buf[2048];
    iso_fs *fs;
    uint32_t first_sector;
    uint32_t position;
    uint32_t length;
} iso_file;

void iso_setup_fs()
{

}

iso_fs* __iso_mounted[10];
size_t __iso_lastid = 0;

void iso_mount(iso_fs* fs)
{
  char name[50] = "/home/iso-*";
  __iso_mounted[__iso_lastid] = fs;
  char* p = itoa(++__iso_lastid, name + 10, 20);
  *p++ = '/';
  mkdir(name, &root);
}

struct file iso_to_file(iso_file file)
{
  struct file f;
  f.extra = kalloc(sizeof(iso_file), KERN_MEM);
  memcpy(f.extra, &file, sizeof(iso_file));
  f.dname = strdup(file.fs->root_dir_ent.name); // for it to be OK 
  return f;
}

void isofs_to_ext2(iso_fs* fs, filesystem** extfs)
{
  (*extfs)->name = "ISO9660";
  (*extfs)->read = iso_read_ext;
}

// to be aligned to filesystem standards

void iso_init(struct iso_header* hdr)
{
  if(hdr == NULL_PTR) return; 
  hdr->typecode = 0x1;
  memcpy(hdr->ident, "CD001", 5);
  hdr->version = 0x1;
  hdr->unused = 0x0;
  hdr->unused2[0] = 0x0;
}

void iso_write(struct iso_header* hdr)
{
  
}

void iso9660_init()
{
  
}