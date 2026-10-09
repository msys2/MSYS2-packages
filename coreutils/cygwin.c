/* cygwin.c - helper functions unique to Cygwin

   Copyright (C) 2005, 2006, 2008, 2011 Free Software Foundation, Inc.

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

#include <config.h>

#include "cygwin.h"

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

/* Return -1 if PATH is not found, 0 if PATH will not have .exe
   appended (it is possible that a PATH that does not exist still
   returns 0 instead of -1, or fails for a PATH that exists but cannot
   be stat'ed), and positive if PATH has ".exe" automatically appended
   by cygwin (1 if PATH is a symlink, 2 otherwise).  Won't change errno.  */

int
cygwin_spelling (char const *path)
{
  int saved_errno = errno;
  int result = 0; /* Start with assumption that PATH is okay.  */
  size_t len;
  struct stat st1;
  struct stat st2;
  char *path_exe;

  /* If PATH will cause EINVAL or ENAMETOOLONG, treat it as missing.  */
  if (! path || ! *path)
    return -1;
  if (PATH_MAX < (len = strlen (path)))
    return -1;
  /* Don't change spelling if there is a trailing `/' or '.exe'.  */
  if (path[len - 1] == '/'
      || (len > 4 && !strcasecmp (&path[len - 4], ".exe")))
    return 0;
  if (lstat (path, &st1) < 0)
    {
      errno = saved_errno;
      return -1;
    }
  if (S_ISDIR(st1.st_mode))
    {
      errno = saved_errno;
      return 0;
    }
  path_exe = malloca (len + 5); /* adding ".exe" and NUL.  */
  strcat (stpcpy (path_exe, path), ".exe");
  if (lstat (path_exe, &st2) == 0 && st1.st_ino == st2.st_ino)
    result = 1 + !S_ISLNK(st1.st_mode);
  freea (path_exe);

  errno = saved_errno;
  return result;
}
