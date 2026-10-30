/* cygwin.h - helper functions unique to Cygwin

   Copyright (C) 2005, 2006, 2008, 2010, 2011 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Written by Eric Blake.  */

#ifndef CYGWIN_H
# define CYGWIN_H 1

#include "malloca.h"

int cygwin_spelling (char const *);

/* Append ".exe" to char *__NAME_ORIG, where __NAME is either NULL or
   between __NAME_ORIG and the nul terminator.  Both params will be
   evaluated more than once and assigned the new value.  The user must
   later call freea(__NAME).  */
#define CYGWIN_APPEND_EXE(__name, __name_orig)                          \
  __name_orig = __name =                                                \
    strcat (strcpy (malloca (strchr (__name ? __name : __name_orig, '\0') \
                             - (__name_orig) + 5),                      \
                    __name_orig), ".exe")

#endif /* CYGWIN_H */
