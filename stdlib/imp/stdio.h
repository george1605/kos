#ifndef __STDIO_IMP__
#define __STDIO_IMP__
#include "../stdio.h"
#include "../../usermode.h"
#include "atomic.h"
#define ERR_EOF (1 << 8)
#define ERR_NOBUF (2 << 8) // tried to flush but the buffer was NULL
#ifdef __cplusplus
extern "C" {
#endif
struct _IO_FILE;
typedef struct _IO_FILE FILE;

int feof(FILE* fp)
{
    return (fp->_IO_read_ptr == fp->_IO_read_end) || (fp->_IO_write_base == fp->_IO_write_end);
}

int ferror(FILE* fp)
{
    return fp->_flags2;
}

void clearerr(FILE* fp)
{
    fp->_flags2 = (fp->_flags2) & 0xFF; // only the low byte remains (buffer info)
}

static int _get_osperms(char* perms)
{
    int p = 0;
    for(int i = 0;perms[i] != '\0';i++) {
        if(perms[i] == 'r')
            p |= F_READ;
        if(perms[i] == 'w')
            p |= F_WRITE;
        if(perms[i] == 'x')
            p |= F_EXEC;
    }
}

FILE* fopen(char* fname, char* perms)
{
    FILE* fp = malloc(sizeof(FILE));
    int p = _get_osperms(perms);
    fp->_fileno = userm_open(fname, p);
    fp->_flags = p;
    return fp;
}
 
void fclose(FILE* fp)
{
    userm_close(fp->_fileno);
    free(fp);
}

void fread(void* buf, size_t count, size_t size, FILE* file)
{
    size_t bytes = count * size;
    if(bytes > (file->_IO_read_end - file->_IO_read_ptr))
    {
        file->_flags2 = EOF;
        return;
    }
    memcpy(buf, file->_IO_read_ptr, bytes);
    file->_IO_read_ptr += bytes;
}

void fwrite(void* buf, size_t count, size_t size, FILE* file)
{
    size_t bytes = count * size;
    memcpy(file->_IO_write_ptr, buf, bytes);
    file->_IO_write_ptr += bytes;
}

void flockfile(FILE* file)
{
    atomic_acquire(file->_lock);
}

void ftrylockfile(FILE* file)
{
    atomic_t atom = (atomic_t*)file->_lock;
    if(atom->locked)
        return;
    flockfile(file);
}

void funlockfile(FILE* file)
{
    atomic_release(file->_lock);
}

int fileno(FILE* file)
{
    if(file == NULL)
        return (int)-1;
    return file->_fileno;
}

void fungetc(int chr, FILE* fp) {
    if(fp->_shortbuf[0] == -1) {
        fp->_shortbuf[0] = chr;
        return;
    }
    if (fp->_IO_read_ptr == fp->_IO_read_base - 1) {
        fp->_flags2 = EOF;
    }
    *(--fp->_IO_read_ptr) = chr;
}

// if char is in the shortbuf (from putc or ungetc)
// then returns it, else reads directly
void fgetc(FILE* fp) {
    if(fp->_shortbuf[0] == -1)
        return *fp->_IO_read_ptr++;
    return fp->_shortbuf[0];
}

void setbuf(FILE* fp, char* buf)
{
    size_t len = strlen(buf);
    if(len < BUFSIZ)
        return;
    fp->_IO_buf_base = buf;
    fp->_IO_buf_end = buf + len;
}

void fflush(FILE* fp)
{
    size_t len = fp->_IO_buf_end - fp->_IO_buf_base;
    userm_write(fp->_fileno, fp->_IO_buf_base, len);
}

size_t ftell(FILE* fp)
{
    if(fp->_flags & F_WRITE)
        return (fp->_IO_write_ptr - fp->_IO_write_base);
    if(fp->_flags & F_READ)
        return (fp->_IO_read_ptr - fp->_IO_read_base);
}

#ifdef _USE_KOSSTD_
#define DEV_ADDR_READ 0x1
#define DEV_ADDR_WRITE 0x2
#define DEV_ADDR_BUF 0x4

FILE* devopen(char* filename)
{
    return fopen(filename, "rw");
}

void devclose(FILE* fp)
{
    fclose(fp);
}

void devsetaddr(FILE* fp, void* addr, size_t size, size_t flags)
{
    if(!(fp->_flags2 & F_WRITE) && (flags & DEV_ADDR_WRITE))
        return;
    if(flags & DEV_ADDR_READ) {
        fp->_IO_buf_base = addr;
        fp->_IO_buf_end = addr + size;
    }

    if(flags & DEV_ADDR_BUF) {
        fp->_IO_read_base = fp->_IO_read_ptr = addr;
        fp->_IO_read_end = addr + size;
    }

    if(flags & DEV_ADDR_WRITE) {
        fp->_IO_write_base = fp->_IO_write_ptr = addr;
        fp->_IO_write_end = addr + size;
    }
}

void devctl(FILE* file, int cmd, size_t arg)
{
    userm_ioctl(file->_fileno, cmd, arg);
}

// grab from the buffer or trigger an IOCTL
int devgetc(FILE* file)
{
    int c;
    if(file->_IO_read_ptr != NULL)
    {
        if(file->_IO_read_ptr == file->_IO_read_end)
            return EOF;
        c = *file->_IO_read_ptr++;
        return c; 
    }
    devctl(file, DEVGETC, &c);
    return c;
}

int devputc(FILE* fp, int c)
{
   if(file->_IO_write_ptr != NULL)
    {
        if(file->_IO_write_ptr == file->_IO_write_end)
            return EOF;
        *file->_IO_write_ptr++ = c;
        return 0; 
    }
    devctl(file, DEVSETC, c);
    return 0;
}

void devstat(FILE* file, struct stat* stat)
{
    devctl(file->_fileno, DEVSTAT, stat);
}

#endif

#ifdef __cplusplus
}
#endif

#endif