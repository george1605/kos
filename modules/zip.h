#pragma once
#include "../fs.h"
#include "../smem.h"

typedef struct {
  char* dir;
  char* name;
  uint32_t* ptr;
  uint64_t size;
  uint32_t flags;
} file_zip_info;

typedef struct {
  file_zip_info* files;
} fs_zip_view; // view of the ZIP sub-fs

static unsigned int read_be32(const unsigned char *p)
{
	return ((unsigned int) p[0] << 24)
	     | ((unsigned int) p[1] << 16)
	     | ((unsigned int) p[2] << 8)
	     | ((unsigned int) p[3]);
}

int tinf_gzip_uncompress(void *dest, unsigned int *destLen,
                         const void *source, unsigned int sourceLen)
{
	const unsigned char *src = (const unsigned char *) source;
	unsigned char *dst = (unsigned char *) dest;
	const unsigned char *start;
	unsigned int dlen, crc32;
	int res;
	unsigned char flg;

	/* -- Check header -- */

	/* Check room for at least 10 byte header and 8 byte trailer */
	if (sourceLen < 18) {
		return -1;
	}

	/* Check id bytes */
	if (src[0] != 0x1F || src[1] != 0x8B) {
		return -1;
	}

	/* Check method is deflate */
	if (src[2] != 8) {
		return -1;
	}

	/* Get flag byte */
	flg = src[3];

	/* Check that reserved bits are zero */
	if (flg & 0xE0) {
		return -1;
	}

	/* -- Find start of compressed data -- */

	/* Skip base header of 10 bytes */
	start = src + 10;

	/* Skip extra data if present */
	if (flg & FEXTRA) {
		unsigned int xlen = read_le16(start);

		if (xlen > sourceLen - 12) {
			return -1;
		}

		start += xlen + 2;
	}

	/* Skip file name if present */
	if (flg & FNAME) {
		do {
			if (start - src >= sourceLen) {
				return -1;
			}
		} while (*start++);
	}

	/* Skip file comment if present */
	if (flg & FCOMMENT) {
		do {
			if (start - src >= sourceLen) {
				return -1;
			}
		} while (*start++);
	}

	/* Check header crc if present */
	if (flg & FHCRC) {
		unsigned int hcrc;

		if (start - src > sourceLen - 2) {
			return -1;
		}

		hcrc = read_le16(start);

		if (hcrc != (tinf_crc32(src, start - src) & 0x0000FFFF)) {
			return -1;
		}

		start += 2;
	}

	/* -- Get decompressed length -- */

	dlen = read_le32(&src[sourceLen - 4]);

	if (dlen > *destLen) {
		return TINF_BUF_ERROR;
	}

	/* -- Get CRC32 checksum of original data -- */

	crc32 = read_le32(&src[sourceLen - 8]);

	/* -- Decompress data -- */

	if ((src + sourceLen) - start < 8) {
		return -1;
	}

	res = tinf_uncompress(dst, destLen, start,
	                      (src + sourceLen) - start - 8);

	if (res != 0) {
		return -1;
	}

	if (*destLen != dlen) {
		return -1;
	}

	/* -- Check CRC32 checksum -- */

	if (crc32 != tinf_crc32(dst, dlen)) {
		return -1;
	}

	return 0;
}

int tinf_zlib_uncompress(void *dest, unsigned int *destLen,
                         const void *source, unsigned int sourceLen)
{
	const unsigned char *src = (const unsigned char *) source;
	unsigned char *dst = (unsigned char *) dest;
	unsigned int a32;
	int res;
	unsigned char cmf, flg;
	if (sourceLen < 6) {
		return -1;
	}

	cmf = src[0];
	flg = src[1];

	if ((256 * cmf + flg) % 31) {
		return -1;
	}

	if ((cmf & 0x0F) != 8) {
		return -1;
	}

	if ((cmf >> 4) > 7) {
		return -1;
	}

	if (flg & 0x20) {
		return -1;
	}

	a32 = read_be32(&src[sourceLen - 4]);
	res = tinf_uncompress(dst, destLen, src + 2, sourceLen - 6);

	if (res != 0) {
		return -1;
	}

	if (a32 != tinf_adler32(dst, *destLen)) {
		return -1;
	}

	return 0;
}