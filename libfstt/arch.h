/*
 * Architecture specifics, little endian 32bit
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
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef LIBFSTT_ARCH_H
#define LIBFSTT_ARCH_H

// environment specific

#ifdef __i386__
#  define MSB_BYTE_FIRST	0	// MSB=1, LSB=0
#  define MSB_BIT_FIRST		0	// MSB=2, LSB=0
#  define FSBYTEORDER		'l'	// 'l' little endian, 'B' big endian
#  define LOGSLP		5	// default SCANLINEPAD is 32bits = 1<<5
					// change it to 3 for some font clients
#else
#  define MSB_BYTE_FIRST	1
#  define MSB_BIT_FIRST		2
#  define FSBYTEORDER		'B'
#  define LOGSLP		5
#endif

// architecture specific

typedef unsigned char	U8;
typedef unsigned short	U16;
typedef unsigned int	U32;

typedef signed char	S8;
typedef signed short	S16;
typedef signed int	S32;

#include <netinet/in.h>

// byte swapping (doing it by ntohl/ntohs works only on little endian CPUs)
#define bswapl(x)	ntohl(x)
#define bswaps(x)	ntohs(x)

#ifdef __i386__
// MULDIV is worth putting in assembler,
// undefine it to use the generic method
#  define MULDIV muldiv
inline int muldiv(int a, int b, int c)
{
	int r;
	// 32Bit * 32Bit / 32Bit with intermediate 64Bit divisor
	__asm__("imull %2\n\tidiv %3, %0" :
		"=a"(r) : "0"(a), "d"(b), "g"(c));
	return r;
}
#endif


#ifndef MULDIV
// 64bit types are only needed for temporary MULDIV results
#    ifdef __alpha
typedef long			S64;
typedef unsigned long		U64;
#    else
typedef long long		S64;
typedef unsigned long long	U64;
#    endif
#endif

//=========== add special cases here ==================

#ifdef __sgi
#  undef LOGSLP
#  define LOGSLP 3
#endif

#endif

