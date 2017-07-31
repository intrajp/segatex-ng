/*
   main.h - header file for main.c 
   This file contains the contents of segatex-ng.

   Copyright (C) 2017 Shintaro Fujiwara

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA
*/

#ifndef SEGATEXD_MAIN_H
#define SEGATEXD_MAIN_H

/* this is used to add extra format checking to the function calls as if this
   was a printf()-like function */
#define LIKE_PRINTF(format_idx, arg_idx) /* no attribute */

/* this is a function of libselinux */
int is_selinux_enabled();

/* this is a function of libaudit */
int audit_is_enabled(int fd);

#endif