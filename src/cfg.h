/*
   cfg.h - definition of configuration information
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

#ifndef CFG_H
#define CFG_H

struct segatex_ng_config
{
    /* the number of threads to start */
    int sgx_threads;
};

/* this is a pointer to the global configuration, it should be available
   once cfg_init() was called */
extern struct segatex_ng_config *segatex_cfg;

/* Initialize the configuration in segatex_cfg. This method
   will read the default configuration file and call exit()
   if an error occurs. */
/* This function could be called anywhere, so declared here. */
void cfg_init(const char *fname);

#endif /* CFG_H */
