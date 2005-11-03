/*
 * Utilities for efficient access to the TTFfile
 *
 * $Id$
 *
 * Copyright (C) 1997-1998 Herbert Duerr
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Softaware
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

//#define DEBUG 1
#include "ttf.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stddef.h>
#include <stdlib.h>

void *
allocMem(int size)
{
	void *ptr;

	ptr = malloc(size);

	return ptr;
}

void *
shrinkMem(void *ptr, int newsize)
{
	ptr = realloc(ptr, newsize);

	return ptr;
}

void
deallocMem(void *ptr, int size)
{
	free(ptr);
}

#ifdef MEMDEBUG

#define MEMPTRS 8192

typedef struct {
	void *ptr;
	int len;
} memstruct;

static memstruct memdbg[MEMPTRS];
static int memidx = 0;
static int memcount = 0;
static int memused = 0;

void
cleanupMem()
{
	debug("Memory holes:\n");
	for (int i = 0; i < memidx; ++i)
		if (memdbg[i].ptr)
			debug("MEM hole[%3d] = %p\n", i, memdbg[i].ptr);

	if (memcount != 0)
		debug("MEM hole: memcount = %d\n", memcount);
}

void *
operator new[](size_t size)
{
	void *ptr = malloc(size);

	memused += size;

	debug("MEM new[](%5d) = %p", size, ptr);
	debug(", memcount = %d, memidx = %d", ++memcount, memidx);

	int i = memidx;

	while (--i >= 0 && memdbg[i].ptr);
	if (i <= 0)
		i = memidx++;

	debug(", idx = %d, used %d\n", i, memused);

	memdbg[i].ptr = ptr;
	memdbg[i].len = size;

	return ptr;
}

void
operator delete[](void *ptr)
{
	debug("MEM delete[](%p)", ptr);
	debug(", memcount = %d, memidx = %d\n", --memcount, memidx);

	int i = memidx;
	while (--i >= 0 && memdbg[i].ptr != ptr);
	if (i >= 0) {
		memdbg[i].ptr = 0;
		memused -= memdbg[i].len;
		debug(", idx = %d, used %d\n", i, memused);
		if (++i == memidx)
			--memidx;
	} else
		debug("Cannot delete!\n");

	free(ptr);
}

void *
operator new(size_t size)
{
	void *ptr = malloc(size);
	memused += size;

	debug("MEM new(%7d) = %p", size, ptr);
	debug(", memcount = %d, memidx = %d", ++memcount, memidx);

	int i = memidx;
	while (--i >= 0 && memdbg[i].ptr);
	if (i <= 0)
		i = memidx++;

	debug(", idx = %d, used %d\n", i, memused);

	memdbg[i].ptr = ptr;
	memdbg[i].len = size;

	return ptr;
}

void
operator delete(void *ptr)
{
	debug("MEM delete(%p)", ptr);
	debug(", memcount = %d, memidx = %d", --memcount, memidx);

	int i = memidx;
	while (--i >= 0 && memdbg[i].ptr != ptr);
	if (i >= 0) {
		memdbg[i].ptr = 0;
		memused -= memdbg[i].len;
		debug(", idx = %d, used %d\n", i, memused);
		if (++i == memidx)
			--memidx;
	} else
		debug("Cannot delete!\n");

	free(ptr);
}

#endif /* MEMDEBUG */


RandomAccessFile::RandomAccessFile(char *fileName)
{
	int fd = open(fileName, O_RDONLY);
	if (fd <= 0) {
		debug("Cannot open \"%s\"\n", fileName);
		ptr = base = 0;
		return;
	}
	struct stat st;
	fstat(fd, &st);
	length = st.st_size;
	base = (u8_t *)mmap(0L, length, PROT_READ, MAP_SHARED, fd, 0L);
	close(fd);
	ptr = absbase = base;
}

void
RandomAccessFile::closeRAFile()
{
	if (absbase && absbase == base && length > 0) {
		munmap(base, length);
	}
}

u32_t
RandomAccessFile::calcChecksum()
{
	u32_t checksum = 0;
	u8_t *saveptr = ptr;

	for (int len = length >> 2; --len >= 0;)
		checksum += readUInt();
	if (length & 3)
		checksum += readUInt() & (-1 << ((-length & 3) << 3));
	ptr = saveptr;

	debug("Checksum is %08X\n", calcChecksum());

	return checksum;
}

