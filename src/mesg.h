/*
 * Message handling routines
 *
 * $Id$
 *
 * Copyright (C) 2004 Guillem Jover
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation;
 * version 2 of the License.
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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <cstdio>

#define error(format, ...)	\
	std::fprintf(stderr, format, ##__VA_ARGS__)
#define warning(format, ...)	\
	std::fprintf(stderr, format, ##__VA_ARGS__)
#define info(format, ...)	\
	std::fprintf(stdout, format, ##__VA_ARGS__)

#endif

