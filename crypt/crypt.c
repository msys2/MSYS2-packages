#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encrypt.h"

const char *sc = "./"
                 "01234567890"
                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                 "abcdefghijklmnopqrstuvwxyz";

int
main (int argc, const char **argv)
{
  char salt[3];

  if (argc == 2)
    {
      srand (time (NULL));
      salt[0] = sc[(rand() & 0x7e) >> 1];
      salt[1] = sc[(rand() & 0x7e) >> 1];
      salt[2] = '\0';
      printf ("%s\n", crypt (argv[1], salt));
      return 0;
    }
  if (argc == 3)
    {
      salt[0] = '\0';
      strncat (salt, argv[1], 2);
      printf ("%s\n", crypt (argv[2], salt));
      return 0;
    }
  fprintf (stderr, "usage: %s passwd\n"
                   "       %s salt passwd\n", argv[0], argv[0]);
  return 1;
}
