/*
 *  cfg.h - definition of configuration information
 *  This file contains the contents of segatex-ng.
 *
 *  Copyright (C) 2014 Arthur de Jong
 *  Copyright (C) 2017-2018 Shintaro Fujiwara
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301 USA
*/

#ifndef SEGATEXD__CFG_H
#define SEGATEXD__CFG_H

struct segatex_ng_config
{
    /* the number of threads to start */
    int threads;
    char send_file [ 255 ];
};

/*  this is a pointer to the global configuration, it should be available
 *  once cfg_init() was called
*/

extern struct segatex_ng_config *segatexd_cfg;

/*  Initialize the configuration in segatexd_cfg. This method
 *  will read the default configuration file and call exit()
 *  if an error occurs.
*/

void cfg_init ( const char *fname );

/* This function reads config file. */

void cfg_read ( const char *fname, struct segatex_ng_config *cfg );

/*  This function frees the memory which had been allocated
 *  for config file.
*/

void cfg_clear (  );

#endif /* SEGATEXD__CFG_H */
