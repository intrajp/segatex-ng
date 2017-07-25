/*
   segatexd.h - definition of segatexd information
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

#ifndef SEGATEXD_H
#define SEGATEXD_H

#ifdef GLOBAL_VALUE_DEFINE
    #define GLOBAL
    #define GLOBAL_VAL(v) = (v)
#else
    #define GLOBAL extern
    #define GLOBAL_VAL(v)
#endif

/*this difinition should be 0*/
GLOBAL int SIG_VALUE; 

/* configuration file of this library */
static const char const *fname = "/etc/segatexd.conf";

#endif /* SEGATEXD_H */
