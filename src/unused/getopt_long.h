/*
   getopt_long.h - definition of getopt_long() for systems that lack it

   Copyright (C) 2001, 2002, 2008, 2010, 2012 Arthur de Jong
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

#ifndef GETOPT_LONG_H
#define GETOPT_LONG_H 1

#ifndef HAVE_GETOPT_LONG

#define no_argument            0
#define required_argument      1
#define optional_argument      2

struct option {
  const char *name;
  int has_arg;
  int *flag;
  int val;
};

/* this is a (poor) getopt_long() replacement for systems that don't have it
   (this is generaly a GNU extention)
   this implementation is by no meens flawless, especialy the optional arguments
   to options and options following filenames is not quite right, allso
   minimal error checking
   */
int getopt_long(int argc, char *const argv[],
                const char *optstring,
                const struct option *longopts, int *longindex);

#endif /* not HAVE_GETOPT_LONG */

#endif /* GETOPT_LONG_H */