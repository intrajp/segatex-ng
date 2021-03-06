/* private.h -- 
 * This file contains the contents of segatex-ng.
 * 
 *  Copyright 2005,2006,2009,2013-14 Red Hat Inc., Durham, North Carolina.
 *  Copyright(C) 2017-2018 Shintaro Fujiwara
 *  All Rights Reserved.
 * 
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 * 
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 * 
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 *  Authors:
 * 	Steve Grubb <sgrubb@redhat.com>
 * 	Shintaro Fujiwara <shintaro.fujiwara@gmail.com>
*/

#ifndef SEGATEXD__PRIVATE_H_
#define SEGATEXD__PRIVATE_H_

#include "dso.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { REAL_ERR, HIDE_IT } hide_t;

/* This structure is for protocol reference only.  All fields are
 *  packed and in network order (LSB first).
*/

struct segatexd_remote_message_wrapper {
	/* The magic number shall never have LF (0x0a) as one of its bytes.  */
	uint32_t	magic;
	/* Bumped when the layout of this structure changes.  */
	uint8_t		header_version;
	/* The minimum support needed to understand this message type.
	 * Normally zero.  */
	uint8_t		message_version;
	/* Upper 8 bits are generic type, see below.  */
	uint32_t	type;
	/* Number of bytes that follow this header  Must be 0..MAX_SEGATEX_MESSAGE_LENGTH.  */
	uint16_t	length;
	/* Copied from message to its reply. */
	uint32_t	sequence_id;
	/* The message follows for LENGTH bytes.  */
};

#define SEGATEX_RMW_HEADER_SIZE		16
/* The magic number shall never have LF (0x0a) as one of its bytes.  */
#define SEGATEX_RMW_MAGIC			0xff0000feUL

#define SEGATEX_RMW_HEADER_VERSION	0

/* If set, this is a reply.  */
#define SEGATEX_RMW_TYPE_REPLYMASK	0x40000000
/* If set, this reply indicates a fatal error of some sort.  */
#define SEGATEX_RMW_TYPE_FATALMASK	0x20000000
/* If set, this reply indicates success but with some warnings.  */
#define SEGATEX_RMW_TYPE_WARNMASK		0x10000000
/* This part of the message type is the details for the above.  */
#define SEGATEX_RMW_TYPE_DETAILMASK	0x0fffffff

/* Version 0 messages.  */
#define SEGATEX_RMW_TYPE_MESSAGE		0x00000000
#define SEGATEX_RMW_TYPE_HEARTBEAT	0x00000001
#define SEGATEX_RMW_TYPE_ACK		0x40000000
#define SEGATEX_RMW_TYPE_ENDING		0x40000001
#define SEGATEX_RMW_TYPE_DISKLOW		0x50000001
#define SEGATEX_RMW_TYPE_DISKFULL		0x60000001
#define SEGATEX_RMW_TYPE_DISKERROR	0x60000002

/* These next four should not be called directly.  */
#define _SEGATEX_RMW_PUTN32(header,i,v)	\
	header[i] = v & 0xff;		\
	header[i+1] = (v>>8) & 0xff;	\
	header[i+2] = (v>>16) & 0xff;	\
	header[i+3] = (v>>24) & 0xff;
#define _SEGATEX_RMW_PUTN16(header,i,v)			\
	header[i] = v & 0xff;		\
	header[i+1] = (v>>8) & 0xff;
#define _SEGATEX_RMW_GETN32(header,i)			\
	(((uint32_t)(header[i] & 0xFF)) |               \
	 (((uint32_t)(header[i+1] & 0xFF))<<8) |        \
	 (((uint32_t)(header[i+2] & 0xFF ))<<16) |      \
	 (((uint32_t)(header[i+3] & 0xFF))<<24))
#define _SEGATEX_RMW_GETN16(header,i)			\
	((uint32_t)(header[i] & 0xFF) | ((uint32_t)(header[i+1] & 0xFF)<<8))

/* For these, HEADER must by of type "unsigned char *" or "unsigned
   char []" */

#define SEGATEX_RMW_PACK_HEADER(header,mver,type,len,seq) \
	_SEGATEX_RMW_PUTN32 (header,0, SEGATEX_RMW_MAGIC); \
	header[4] = SEGATEX_RMW_HEADER_VERSION;  \
	header[5] = mver;  \
	_SEGATEX_RMW_PUTN32 (header,6, type);  \
	_SEGATEX_RMW_PUTN16 (header,10, len); \
	_SEGATEX_RMW_PUTN32 (header,12, seq);

#define SEGATEX_RMW_IS_MAGIC(header,length)		\
	(length >= 4 && _SEGATEX_RMW_GETN32 (header,0) == SEGATEX_RMW_MAGIC)

#define SEGATEX_RMW_UNPACK_HEADER(header,hver,mver,type,len,seq) \
	hver = header[4]; \
	mver = header[5]; \
	type = _SEGATEX_RMW_GETN32 (header,6); \
	len = _SEGATEX_RMW_GETN16 (header,10); \
	seq = _SEGATEX_RMW_GETN32 (header,12);

/* General */
/* Internal syslog messaging */
void segatex_msg ( int priority, const char *fmt, ... ) 
#ifdef __GNUC__
	__attribute__ ((format (printf, 2, 3)));
#else
	;
#endif

extern int segatex_send (int fd, int type, const void *data, unsigned int size );

SEGATEX_HIDDEN_START

// This is the main messaging function used internally
extern int segatex_send_user_message(int fd, int type, hide_t hide_err, 
	const char *message);

SEGATEX_HIDDEN_END

// strsplit.c
char *segatex_strsplit_r(char *s, char **savedpp);
char *segatex_strsplit(char *s);

// libsegatex.c
extern int _segatex_permadded;
extern int _segatex_archadded;
extern int _segatex_syscalladded;
extern int _segatex_exeadded;
extern unsigned int _segatex_elf;

#ifdef __cplusplus
}
#endif

#endif

