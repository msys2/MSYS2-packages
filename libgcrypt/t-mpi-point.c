/* t-mpi-point.c  - Tests for mpi point functions
 * Copyright (C) 2013 g10 Code GmbH
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#define PGM "t-mpi-point"
#include "t-common.h"

static struct
{
  const char *desc;           /* Description of the curve.  */
  const char *p;              /* Order of the prime field.  */
  const char *a, *b;          /* The coefficients. */
  const char *n;              /* The order of the base point.  */
  const char *g_x, *g_y;      /* Base point.  */
  const char *h;              /* Cofactor.  */
} test_curve[] =
  {
    {
      "NIST P-224",
      "0xffffffffffffffffffffffffffffffff000000000000000000000001",
      "0xfffffffffffffffffffffffffffffffefffffffffffffffffffffffe",
      "0xb4050a850c04b3abf54132565044b0b7d7bfd8ba270b39432355ffb4",
      "0xffffffffffffffffffffffffffff16a2e0b8f03e13dd29455c5c2a3d" ,

      "0xb70e0cbd6bb4bf7f321390b94a03c1d356c21122343280d6115c1d21",
      "0xbd376388b5f723fb4c22dfe6cd4375a05a07476444d5819985007e34",
      "0x01"
    },
    {
      "NIST P-256",
      "0xffffffff00000001000000000000000000000000ffffffffffffffffffffffff",
      "0xffffffff00000001000000000000000000000000fffffffffffffffffffffffc",
      "0x5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b",
      "0xffffffff00000000ffffffffffffffffbce6faada7179e84f3b9cac2fc632551",

      "0x6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",
      "0x4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5",
      "0x01"
    },
    {
      "NIST P-384",
      "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe"
      "ffffffff0000000000000000ffffffff",
      "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe"
      "ffffffff0000000000000000fffffffc",
      "0xb3312fa7e23ee7e4988e056be3f82d19181d9c6efe8141120314088f5013875a"
      "c656398d8a2ed19d2a85c8edd3ec2aef",
      "0xffffffffffffffffffffffffffffffffffffffffffffffffc7634d81f4372ddf"
      "581a0db248b0a77aecec196accc52973",

      "0xaa87ca22be8b05378eb1c71ef320ad746e1d3b628ba79b9859f741e082542a38"
      "5502f25dbf55296c3a545e3872760ab7",
      "0x3617de4a96262c6f5d9e98bf9292dc29f8f41dbd289a147ce9da3113b5f0b8c0"
      "0a60b1ce1d7e819d7a431d7c90ea0e5f",
      "0x01"
    },
    {
      "NIST P-521",
      "0x01ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
      "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
      "0x01ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
      "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc",
      "0x051953eb9618e1c9a1f929a21a0b68540eea2da725b99b315f3b8b489918ef10"
      "9e156193951ec7e937b1652c0bd3bb1bf073573df883d2c34f1ef451fd46b503f00",
      "0x1fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
      "ffa51868783bf2f966b7fcc0148f709a5d03bb5c9b8899c47aebb6fb71e91386409",

      "0xc6858e06b70404e9cd9e3ecb662395b4429c648139053fb521f828af606b4d3d"
      "baa14b5e77efe75928fe1dc127a2ffa8de3348b3c1856a429bf97e7e31c2e5bd66",
      "0x11839296a789a3bc0045c8a5fb42c7d1bd998f54449579b446817afbd17273e6"
      "62c97ee72995ef42640c550b9013fad0761353c7086a272c24088be94769fd16650",
      "0x01"
    },
    {
      "Ed25519",
      "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED",
      "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEC",
      "0x52036CEE2B6FFE738CC740797779E89800700A4D4141D8AB75EB4DCA135978A3",
      "0x1000000000000000000000000000000014DEF9DEA2F79CD65812631A5CF5D3ED",
      "0x216936D3CD6E53FEC0A4E231FDD6DC5C692CC7609525A7B2C9562D608F25D51A",
      "0x6666666666666666666666666666666666666666666666666666666666666658",
      "0x08"
    },
    { NULL, NULL, NULL, NULL, NULL, NULL }
  };

/* A sample public key for NIST P-256.  */
static const char sample_p256_q[] =
  "04"
  "42B927242237639A36CE9221B340DB1A9AB76DF2FE3E171277F6A4023DED146E"
  "E86525E38CCECFF3FB8D152CC6334F70D23A525175C1BCBDDE6E023B2228770E";
static const char sample_p256_q_x[] =
  "42B927242237639A36CE9221B340DB1A9AB76DF2FE3E171277F6A4023DED146E";
static const char sample_p256_q_y[] =
  "00E86525E38CCECFF3FB8D152CC6334F70D23A525175C1BCBDDE6E023B2228770E";


/* A sample public key for Ed25519.  */
static const char sample_ed25519_q[] =
  "04"
  "55d0e09a2b9d34292297e08d60d0f620c513d47253187c24b12786bd777645ce"
  "1a5107f7681a02af2523a6daf372e10e3a0764c9d3fe4bd5b70ab18201985ad7";
static const char sample_ed25519_q_x[] =
  "55d0e09a2b9d34292297e08d60d0f620c513d47253187c24b12786bd777645ce";
static const char sample_ed25519_q_y[] =
  "1a5107f7681a02af2523a6daf372e10e3a0764c9d3fe4bd5b70ab18201985ad7";
static const char sample_ed25519_q_eddsa[] =
  "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a";
static const char sample_ed25519_d[] =
  "9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60";


static void
print_mpi_2 (const char *text, const char *text2, gcry_mpi_t a)
{
  gcry_error_t err;
  char *buf;
  void *bufaddr = &buf;

  err = gcry_mpi_aprint (GCRYMPI_FMT_HEX, bufaddr, NULL, a);
  if (err)
    fprintf (stderr, "%s%s: [error printing number: %s]\n",
             text, text2? text2:"", gpg_strerror (err));
  else
    {
      fprintf (stderr, "%s%s: %s\n", text, text2? text2:"", buf);
      gcry_free (buf);
    }
}


static void
print_mpi (const char *text, gcry_mpi_t a)
{
  print_mpi_2 (text, NULL, a);
}


static void
print_point (const char *text, gcry_mpi_point_t a)
{
  gcry_mpi_t x, y, z;

  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);
  gcry_mpi_point_get (x, y, z, a);
  print_mpi_2 (text, ".x", x);
  print_mpi_2 (text, ".y", y);
  print_mpi_2 (text, ".z", z);
  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);
}


static void
print_sexp (const char *prefix, gcry_sexp_t a)
{
  char *buf;
  size_t size;

  if (prefix)
    fputs (prefix, stderr);
  size = gcry_sexp_sprint (a, GCRYSEXP_FMT_ADVANCED, NULL, 0);
  buf = gcry_xmalloc (size);

  gcry_sexp_sprint (a, GCRYSEXP_FMT_ADVANCED, buf, size);
  fprintf (stderr, "%.*s", (int)size, buf);
  gcry_free (buf);
}


static gcry_mpi_t
hex2mpi (const char *string)
{
  gpg_error_t err;
  gcry_mpi_t val;

  err = gcry_mpi_scan (&val, GCRYMPI_FMT_HEX, string, 0, NULL);
  if (err)
    die ("hex2mpi '%s' failed: %s\n", string, gpg_strerror (err));
  return val;
}


/* Convert STRING consisting of hex characters into its binary
   representation and return it as an allocated buffer. The valid
   length of the buffer is returned at R_LENGTH.  The string is
   delimited by end of string.  The function returns NULL on
   error.  */
static void *
hex2buffer (const char *string, size_t *r_length)
{
  const char *s;
  unsigned char *buffer;
  size_t length;

  buffer = xmalloc (strlen(string)/2+1);
  length = 0;
  for (s=string; *s; s +=2 )
    {
      if (!hexdigitp (s) || !hexdigitp (s+1))
        return NULL;           /* Invalid hex digits. */
      ((unsigned char*)buffer)[length++] = xtoi_2 (s);
    }
  *r_length = length;
  return buffer;
}


static gcry_mpi_t
hex2mpiopa (const char *string)
{
  char *buffer;
  size_t buflen;
  gcry_mpi_t val;

  buffer = hex2buffer (string, &buflen);
  if (!buffer)
    die ("hex2mpiopa '%s' failed: parser error\n", string);
  val = gcry_mpi_set_opaque (NULL, buffer, buflen*8);
  if (!buffer)
    die ("hex2mpiopa '%s' failed: set_opaque error\n", string);
  return val;
}


/* Compare A to B, where B is given as a hex string.  */
static int
cmp_mpihex (gcry_mpi_t a, const char *b)
{
  gcry_mpi_t bval;
  int res;

  if (gcry_mpi_get_flag (a, GCRYMPI_FLAG_OPAQUE))
    bval = hex2mpiopa (b);
  else
    bval = hex2mpi (b);
  res = gcry_mpi_cmp (a, bval);
  gcry_mpi_release (bval);
  return res;
}


/* Wrapper to emulate the libgcrypt internal EC context allocation
   function.  */
static gpg_error_t
ec_p_new (gcry_ctx_t *r_ctx, gcry_mpi_t p, gcry_mpi_t a)
{
  gpg_error_t err;
  gcry_sexp_t sexp;

  if (p && a)
    err = gcry_sexp_build (&sexp, NULL, "(ecdsa (p %m)(a %m))", p, a);
  else if (p)
    err = gcry_sexp_build (&sexp, NULL, "(ecdsa (p %m))", p);
  else if (a)
    err = gcry_sexp_build (&sexp, NULL, "(ecdsa (a %m))", a);
  else
    err = gcry_sexp_build (&sexp, NULL, "(ecdsa)");
  if (err)
    return err;
  err = gcry_mpi_ec_new (r_ctx, sexp, NULL);
  gcry_sexp_release (sexp);
  return err;
}



static void
set_get_point (void)
{
  gcry_mpi_point_t point, point2;
  gcry_mpi_t x, y, z;

  wherestr = "set_get_point";
  info ("checking point setting functions\n");

  point = gcry_mpi_point_new (0);
  x = gcry_mpi_set_ui (NULL, 17);
  y = gcry_mpi_set_ui (NULL, 42);
  z = gcry_mpi_set_ui (NULL, 11371);
  gcry_mpi_point_get (x, y, z, point);
  if (gcry_mpi_cmp_ui (x, 0)
      || gcry_mpi_cmp_ui (y, 0) || gcry_mpi_cmp_ui (z, 0))
    fail ("new point not initialized to (0,0,0)\n");
  gcry_mpi_point_snatch_get (x, y, z, point);
  point = NULL;
  if (gcry_mpi_cmp_ui (x, 0)
      || gcry_mpi_cmp_ui (y, 0) || gcry_mpi_cmp_ui (z, 0))
    fail ("snatch_get failed\n");
  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);

  point = gcry_mpi_point_new (0);
  x = gcry_mpi_set_ui (NULL, 17);
  y = gcry_mpi_set_ui (NULL, 42);
  z = gcry_mpi_set_ui (NULL, 11371);
  gcry_mpi_point_set (point, x, y, z);
  gcry_mpi_set_ui (x, 23);
  gcry_mpi_set_ui (y, 24);
  gcry_mpi_set_ui (z, 25);
  gcry_mpi_point_get (x, y, z, point);
  if (gcry_mpi_cmp_ui (x, 17)
      || gcry_mpi_cmp_ui (y, 42) || gcry_mpi_cmp_ui (z, 11371))
    fail ("point_set/point_get failed\n");
  gcry_mpi_point_snatch_set (point, x, y, z);
  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);
  gcry_mpi_point_get (x, y, z, point);
  if (gcry_mpi_cmp_ui (x, 17)
      || gcry_mpi_cmp_ui (y, 42) || gcry_mpi_cmp_ui (z, 11371))
    fail ("point_snatch_set/point_get failed\n");

  point2 = gcry_mpi_point_copy (point);

  gcry_mpi_point_get (x, y, z, point2);
  if (gcry_mpi_cmp_ui (x, 17)
      || gcry_mpi_cmp_ui (y, 42) || gcry_mpi_cmp_ui (z, 11371))
    fail ("point_copy failed (1)\n");

  gcry_mpi_point_release (point);

  gcry_mpi_point_get (x, y, z, point2);
  if (gcry_mpi_cmp_ui (x, 17)
      || gcry_mpi_cmp_ui (y, 42) || gcry_mpi_cmp_ui (z, 11371))
    fail ("point_copy failed (2)\n");

  gcry_mpi_point_release (point2);

  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);
}


static void
context_alloc (void)
{
  gpg_error_t err;
  gcry_ctx_t ctx;
  gcry_mpi_t p, a;

  wherestr = "context_alloc";
  info ("checking context functions\n");

  p = gcry_mpi_set_ui (NULL, 1);
  a = gcry_mpi_set_ui (NULL, 1);
  err = ec_p_new (&ctx, p, a);
  if (err)
    die ("ec_p_new returned an error: %s\n", gpg_strerror (err));
  gcry_mpi_release (p);
  gcry_mpi_release (a);
  gcry_ctx_release (ctx);

  p = NULL;
  a = gcry_mpi_set_ui (NULL, 0);

  err = ec_p_new (&ctx, p, a);
  if (!err || gpg_err_code (err) != GPG_ERR_EINVAL)
    fail ("ec_p_new: bad parameter detection failed (1)\n");

  gcry_mpi_release (a);
  a = NULL;
  err = ec_p_new (&ctx, p, a);
  if (!err || gpg_err_code (err) != GPG_ERR_EINVAL)
    fail ("ec_p_new: bad parameter detection failed (2)\n");

}


static int
get_and_cmp_mpi (const char *name, const char *mpistring, const char *desc,
                 gcry_ctx_t ctx)
{
  gcry_mpi_t mpi;

  mpi = gcry_mpi_ec_get_mpi (name, ctx, 1);
  if (!mpi)
    {
      fail ("error getting parameter '%s' of curve '%s'\n", name, desc);
      return 1;
    }
  if (debug)
    print_mpi (name, mpi);
  if (cmp_mpihex (mpi, mpistring))
    {
      fail ("parameter '%s' of curve '%s' does not match\n", name, desc);
      gcry_mpi_release (mpi);
      return 1;
    }
  gcry_mpi_release (mpi);
  return 0;
}


static int
get_and_cmp_point (const char *name,
                   const char *mpi_x_string, const char *mpi_y_string,
                   const char *desc, gcry_ctx_t ctx)
{
  gcry_mpi_point_t point;
  gcry_mpi_t x, y, z;
  int result = 0;

  point = gcry_mpi_ec_get_point (name, ctx, 1);
  if (!point)
    {
      fail ("error getting point parameter '%s' of curve '%s'\n", name, desc);
      return 1;
    }
  if (debug)
    print_point (name, point);

  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);
  gcry_mpi_point_snatch_get (x, y, z, point);
  if (cmp_mpihex (x, mpi_x_string))
    {
      fail ("x coordinate of '%s' of curve '%s' does not match\n", name, desc);
      result = 1;
    }
  if (cmp_mpihex (y, mpi_y_string))
    {
      fail ("y coordinate of '%s' of curve '%s' does not match\n", name, desc);
      result = 1;
    }
  if (cmp_mpihex (z, "01"))
    {
      fail ("z coordinate of '%s' of curve '%s' is not 1\n", name, desc);
      result = 1;
    }
  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);
  return result;
}


static void
context_param (void)
{
  gpg_error_t err;
  int idx;
  gcry_ctx_t ctx = NULL;
  gcry_mpi_t q, d;
  gcry_sexp_t keyparam;

  wherestr = "context_param";

  info ("checking standard curves\n");
  for (idx=0; test_curve[idx].desc; idx++)
    {
      /* P-192 and Ed25519 are not supported in fips mode */
      if (gcry_fips_mode_active())
        {
          if (!strcmp(test_curve[idx].desc, "NIST P-192")
              || !strcmp(test_curve[idx].desc, "Ed25519"))
            {
	      info ("skipping %s in fips mode\n", test_curve[idx].desc );
              continue;
            }
        }

      gcry_ctx_release (ctx);
      err = gcry_mpi_ec_new (&ctx, NULL, test_curve[idx].desc);
      if (err)
        {
          fail ("can't create context for curve '%s': %s\n",
                test_curve[idx].desc, gpg_strerror (err));
          continue;
        }
      if (get_and_cmp_mpi ("p", test_curve[idx].p, test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_mpi ("a", test_curve[idx].a, test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_mpi ("b", test_curve[idx].b, test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_mpi ("g.x",test_curve[idx].g_x, test_curve[idx].desc,ctx))
        continue;
      if (get_and_cmp_mpi ("g.y",test_curve[idx].g_y, test_curve[idx].desc,ctx))
        continue;
      if (get_and_cmp_mpi ("n", test_curve[idx].n, test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_point ("g", test_curve[idx].g_x, test_curve[idx].g_y,
                             test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_mpi ("h", test_curve[idx].h, test_curve[idx].desc, ctx))
        continue;

    }

  info ("checking sample public key (nistp256)\n");
  q = hex2mpi (sample_p256_q);
  err = gcry_sexp_build (&keyparam, NULL,
                        "(public-key(ecc(curve %s)(q %m)))",
                        "NIST P-256", q);
  if (err)
    die ("gcry_sexp_build failed: %s\n", gpg_strerror (err));
  gcry_mpi_release (q);

  /* We can't call gcry_pk_testkey because it is only implemented for
     private keys.  */
  /* err = gcry_pk_testkey (keyparam); */
  /* if (err) */
  /*   fail ("gcry_pk_testkey failed for sample public key: %s\n", */
  /*         gpg_strerror (err)); */

  gcry_ctx_release (ctx);
  err = gcry_mpi_ec_new (&ctx, keyparam, NULL);
  if (err)
    fail ("gcry_mpi_ec_new failed for sample public key (nistp256): %s\n",
          gpg_strerror (err));
  else
    {
      gcry_sexp_t sexp;

      get_and_cmp_mpi ("q", sample_p256_q, "nistp256", ctx);
      get_and_cmp_point ("q", sample_p256_q_x, sample_p256_q_y, "nistp256",
                         ctx);

      /* Delete Q.  */
      err = gcry_mpi_ec_set_mpi ("q", NULL, ctx);
      if (err)
        fail ("clearing Q for nistp256 failed: %s\n", gpg_strerror (err));
      if (gcry_mpi_ec_get_mpi ("q", ctx, 0))
        fail ("clearing Q for nistp256 did not work\n");

      /* Set Q again.  */
      q = hex2mpi (sample_p256_q);
      err = gcry_mpi_ec_set_mpi ("q", q, ctx);
      if (err)
        fail ("setting Q for nistp256 failed: %s\n", gpg_strerror (err));
      get_and_cmp_mpi ("q", sample_p256_q, "nistp256(2)", ctx);
      gcry_mpi_release (q);

      /* Get as s-expression.  */
      err = gcry_pubkey_get_sexp (&sexp, 0, ctx);
      if (err)
        fail ("gcry_pubkey_get_sexp(0) failed: %s\n", gpg_strerror (err));
      else if (debug)
        print_sexp ("Result of gcry_pubkey_get_sexp (0):\n", sexp);
      gcry_sexp_release (sexp);

      err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_PUBKEY, ctx);
      if (err)
        fail ("gcry_pubkey_get_sexp(GET_PUBKEY) failed: %s\n",
              gpg_strerror (err));
      else if (debug)
        print_sexp ("Result of gcry_pubkey_get_sexp (GET_PUBKEY):\n", sexp);
      gcry_sexp_release (sexp);

      err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_SECKEY, ctx);
      if (gpg_err_code (err) != GPG_ERR_NO_SECKEY)
        fail ("gcry_pubkey_get_sexp(GET_SECKEY) returned wrong error: %s\n",
              gpg_strerror (err));
      gcry_sexp_release (sexp);
    }

  /* Skipping Ed25519 if in FIPS mode (it isn't supported) */
  if (gcry_fips_mode_active())
    goto cleanup;

  info ("checking sample public key (Ed25519)\n");
  q = hex2mpi (sample_ed25519_q);
  gcry_sexp_release (keyparam);
  err = gcry_sexp_build (&keyparam, NULL,
                        "(public-key(ecc(curve %s)(flags eddsa)(q %m)))",
                        "Ed25519", q);
  if (err)
    die ("gcry_sexp_build failed: %s\n", gpg_strerror (err));
  gcry_mpi_release (q);

  /* We can't call gcry_pk_testkey because it is only implemented for
     private keys.  */
  /* err = gcry_pk_testkey (keyparam); */
  /* if (err) */
  /*   fail ("gcry_pk_testkey failed for sample public key: %s\n", */
  /*         gpg_strerror (err)); */

  gcry_ctx_release (ctx);
  err = gcry_mpi_ec_new (&ctx, keyparam, NULL);
  if (err)
    fail ("gcry_mpi_ec_new failed for sample public key: %s\n",
          gpg_strerror (err));
  else
    {
      gcry_sexp_t sexp;

      get_and_cmp_mpi ("q", sample_ed25519_q, "Ed25519", ctx);
      get_and_cmp_point ("q", sample_ed25519_q_x, sample_ed25519_q_y,
                         "Ed25519", ctx);
      get_and_cmp_mpi ("q@eddsa", sample_ed25519_q_eddsa, "Ed25519", ctx);

      /* Set d to see whether Q is correctly re-computed.  */
      d = hex2mpi (sample_ed25519_d);
      err = gcry_mpi_ec_set_mpi ("d", d, ctx);
      if (err)
        fail ("setting d for Ed25519 failed: %s\n", gpg_strerror (err));
      gcry_mpi_release (d);
      get_and_cmp_mpi ("q", sample_ed25519_q, "Ed25519(recompute Q)", ctx);

      /* Delete Q by setting d and then clearing d.  The clearing is
         required so that we can check whether Q has been cleared and
         because further tests only expect a public key.  */
      d = hex2mpi (sample_ed25519_d);
      err = gcry_mpi_ec_set_mpi ("d", d, ctx);
      if (err)
        fail ("setting d for Ed25519 failed: %s\n", gpg_strerror (err));
      gcry_mpi_release (d);
      err = gcry_mpi_ec_set_mpi ("d", NULL, ctx);
      if (err)
        fail ("setting d for Ed25519 failed(2): %s\n", gpg_strerror (err));
      if (gcry_mpi_ec_get_mpi ("q", ctx, 0))
        fail ("setting d for Ed25519 did not reset Q\n");

      /* Set Q again.  We need to use an opaque MPI here because
         sample_ed25519_q is in uncompressed format which can only be
         auto-detected if passed opaque.  */
      q = hex2mpiopa (sample_ed25519_q);
      err = gcry_mpi_ec_set_mpi ("q", q, ctx);
      if (err)
        fail ("setting Q for Ed25519 failed: %s\n", gpg_strerror (err));
      gcry_mpi_release (q);
      get_and_cmp_mpi ("q", sample_ed25519_q, "Ed25519(2)", ctx);

      /* Get as s-expression.  */
      err = gcry_pubkey_get_sexp (&sexp, 0, ctx);
      if (err)
        fail ("gcry_pubkey_get_sexp(0) failed: %s\n", gpg_strerror (err));
      else if (debug)
        print_sexp ("Result of gcry_pubkey_get_sexp (0):\n", sexp);
      gcry_sexp_release (sexp);

      err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_PUBKEY, ctx);
      if (err)
        fail ("gcry_pubkey_get_sexp(GET_PUBKEY) failed: %s\n",
              gpg_strerror (err));
      else if (debug)
        print_sexp ("Result of gcry_pubkey_get_sexp (GET_PUBKEY):\n", sexp);
      gcry_sexp_release (sexp);

      err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_SECKEY, ctx);
      if (gpg_err_code (err) != GPG_ERR_NO_SECKEY)
        fail ("gcry_pubkey_get_sexp(GET_SECKEY) returned wrong error: %s\n",
              gpg_strerror (err));
      gcry_sexp_release (sexp);

    }

 cleanup:
  gcry_ctx_release (ctx);
  gcry_sexp_release (keyparam);
}




/* Create a new point from (X,Y,Z) given as hex strings.  */
gcry_mpi_point_t
make_point (const char *x, const char *y, const char *z)
{
  gcry_mpi_point_t point;

  point = gcry_mpi_point_new (0);
  gcry_mpi_point_snatch_set (point, hex2mpi (x), hex2mpi (y), hex2mpi (z));

  return point;
}


/* This tests checks that the low-level EC API yields the same result
   as using the high level API.  The values have been taken from a
   test run using the high level API.  */
static void
basic_ec_math (void)
{
  gpg_error_t err;
  gcry_ctx_t ctx;
  gcry_mpi_t P, A;
  gcry_mpi_point_t G, Q;
  gcry_mpi_t d;
  gcry_mpi_t x, y, z;

  wherestr = "basic_ec_math";
  info ("checking basic math functions for EC\n");

  P = hex2mpi ("0xfffffffffffffffffffffffffffffffeffffffffffffffff");
  A = hex2mpi ("0xfffffffffffffffffffffffffffffffefffffffffffffffc");
  G = make_point ("188DA80EB03090F67CBF20EB43A18800F4FF0AFD82FF1012",
                  "7192B95FFC8DA78631011ED6B24CDD573F977A11E794811",
                  "1");
  d = hex2mpi ("D4EF27E32F8AD8E2A1C6DDEBB1D235A69E3CEF9BCE90273D");
  Q = gcry_mpi_point_new (0);

  err = ec_p_new (&ctx, P, A);
  if (err)
    die ("ec_p_new failed: %s\n", gpg_strerror (err));

  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);

  {
    /* A quick check that multiply by zero works.  */
    gcry_mpi_t tmp;

    tmp = gcry_mpi_new (0);
    gcry_mpi_ec_mul (Q, tmp, G, ctx);
    gcry_mpi_release (tmp);
    gcry_mpi_point_get (x, y, z, Q);
    if (gcry_mpi_cmp_ui (x, 0) || gcry_mpi_cmp_ui (y, 0)
        || gcry_mpi_cmp_ui (z, 0))
      fail ("multiply a point by zero failed\n");
  }

  gcry_mpi_ec_mul (Q, d, G, ctx);
  gcry_mpi_point_get (x, y, z, Q);
  if (cmp_mpihex (x, "222D9EC717C89D047E0898C9185B033CD11C0A981EE6DC66")
      || cmp_mpihex (y, "605DE0A82D70D3E0F84A127D0739ED33D657DF0D054BFDE8")
      || cmp_mpihex (z, "00B06B519071BC536999AC8F2D3934B3C1FC9EACCD0A31F88F"))
    fail ("computed public key does not match\n");
  if (debug)
    {
      print_mpi ("Q.x", x);
      print_mpi ("Q.y", y);
      print_mpi ("Q.z", z);
    }

  if (gcry_mpi_ec_get_affine (x, y, Q, ctx))
    fail ("failed to get affine coordinates\n");
  if (cmp_mpihex (x, "008532093BA023F4D55C0424FA3AF9367E05F309DC34CDC3FE")
      || cmp_mpihex (y, "00C13CA9E617C6C8487BFF6A726E3C4F277913D97117939966"))
    fail ("computed affine coordinates of public key do not match\n");
  if (debug)
    {
      print_mpi ("q.x", x);
      print_mpi ("q.y", y);
    }

  gcry_mpi_release (z);
  gcry_mpi_release (y);
  gcry_mpi_release (x);
  gcry_mpi_point_release (Q);
  gcry_mpi_release (d);
  gcry_mpi_point_release (G);
  gcry_mpi_release (A);
  gcry_mpi_release (P);
  gcry_ctx_release (ctx);
}


/* This is the same as basic_ec_math but uses more advanced
   features.  */
static void
basic_ec_math_simplified (void)
{
}


/* Check the math used with Twisted Edwards curves.  */
static void
twistededwards_math (void)
{
  gpg_error_t err;
  gcry_ctx_t ctx;
  gcry_mpi_point_t G, Q;
  gcry_mpi_t k;
  gcry_mpi_t w, a, x, y, z, p, n, b, I;

  wherestr = "twistededwards_math";
  info ("checking basic Twisted Edwards math\n");

  err = gcry_mpi_ec_new (&ctx, NULL, "Ed25519");
  if (err)
    die ("gcry_mpi_ec_new failed: %s\n", gpg_strerror (err));

  k = hex2mpi
    ("2D3501E723239632802454EE5DDC406EFB0BDF18486A5BDE9C0390A9C2984004"
     "F47252B628C953625B8DEB5DBCB8DA97AA43A1892D11FA83596F42E0D89CB1B6");
  G = gcry_mpi_ec_get_point ("g", ctx, 1);
  if (!G)
    die ("gcry_mpi_ec_get_point(G) failed\n");
  Q = gcry_mpi_point_new (0);


  w = gcry_mpi_new (0);
  a = gcry_mpi_new (0);
  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);
  I = gcry_mpi_new (0);
  p = gcry_mpi_ec_get_mpi ("p", ctx, 1);
  n = gcry_mpi_ec_get_mpi ("n", ctx, 1);
  b = gcry_mpi_ec_get_mpi ("b", ctx, 1);

  /* Check: 2^{p-1} mod p == 1 */
  gcry_mpi_sub_ui (a, p, 1);
  gcry_mpi_powm (w, GCRYMPI_CONST_TWO, a, p);
  if (gcry_mpi_cmp_ui (w, 1))
    fail ("failed assertion: 2^{p-1} mod p == 1\n");

  /* Check: p % 4 == 1 */
  gcry_mpi_mod (w, p, GCRYMPI_CONST_FOUR);
  if (gcry_mpi_cmp_ui (w, 1))
    fail ("failed assertion: p %% 4 == 1\n");

  /* Check: 2^{n-1} mod n == 1 */
  gcry_mpi_sub_ui (a, n, 1);
  gcry_mpi_powm (w, GCRYMPI_CONST_TWO, a, n);
  if (gcry_mpi_cmp_ui (w, 1))
    fail ("failed assertion: 2^{n-1} mod n == 1\n");

  /* Check: b^{(p-1)/2} mod p == p-1 */
  gcry_mpi_sub_ui (a, p, 1);
  gcry_mpi_div (x, NULL, a, GCRYMPI_CONST_TWO, -1);
  gcry_mpi_powm (w, b, x, p);
  gcry_mpi_abs (w);
  if (gcry_mpi_cmp (w, a))
    fail ("failed assertion: b^{(p-1)/2} mod p == p-1\n");

  /* I := 2^{(p-1)/4} mod p */
  gcry_mpi_sub_ui (a, p, 1);
  gcry_mpi_div (x, NULL, a, GCRYMPI_CONST_FOUR, -1);
  gcry_mpi_powm (I, GCRYMPI_CONST_TWO, x, p);

  /* Check: I^2 mod p == p-1 */
  gcry_mpi_powm (w, I, GCRYMPI_CONST_TWO, p);
  if (gcry_mpi_cmp (w, a))
    fail ("failed assertion: I^2 mod p == p-1\n");

  /* Check: G is on the curve */
  if (!gcry_mpi_ec_curve_point (G, ctx))
    fail ("failed assertion: G is on the curve\n");

  /* Check: nG == (0,1) */
  gcry_mpi_ec_mul (Q, n, G, ctx);
  if (gcry_mpi_ec_get_affine (x, y, Q, ctx))
    fail ("failed to get affine coordinates\n");
  if (gcry_mpi_cmp_ui (x, 0) || gcry_mpi_cmp_ui (y, 1))
    fail ("failed assertion: nG == (0,1)\n");

  /* Now two arbitrary point operations taken from the ed25519.py
     sample data.  */
  gcry_mpi_release (a);
  a = hex2mpi
    ("4f71d012df3c371af3ea4dc38385ca5bb7272f90cb1b008b3ed601c76de1d496"
     "e30cbf625f0a756a678d8f256d5325595cccc83466f36db18f0178eb9925edd3");
  gcry_mpi_ec_mul (Q, a, G, ctx);
  if (gcry_mpi_ec_get_affine (x, y, Q, ctx))
    fail ("failed to get affine coordinates\n");
  if (cmp_mpihex (x, ("157f7361c577aad36f67ed33e38dc7be"
                      "00014fecc2165ca5cee9eee19fe4d2c1"))
      || cmp_mpihex (y, ("5a69dbeb232276b38f3f5016547bb2a2"
                         "4025645f0b820e72b8cad4f0a909a092")))
    {
      fail ("sample point multiply failed:\n");
      print_mpi ("r", a);
      print_mpi ("Rx", x);
      print_mpi ("Ry", y);
    }

  gcry_mpi_release (a);
  a = hex2mpi
    ("2d3501e723239632802454ee5ddc406efb0bdf18486a5bde9c0390a9c2984004"
     "f47252b628c953625b8deb5dbcb8da97aa43a1892d11fa83596f42e0d89cb1b6");
  gcry_mpi_ec_mul (Q, a, G, ctx);
  if (gcry_mpi_ec_get_affine (x, y, Q, ctx))
    fail ("failed to get affine coordinates\n");
  if (cmp_mpihex (x, ("6218e309d40065fcc338b3127f468371"
                      "82324bd01ce6f3cf81ab44e62959c82a"))
      || cmp_mpihex (y, ("5501492265e073d874d9e5b81e7f8784"
                         "8a826e80cce2869072ac60c3004356e5")))
    {
      fail ("sample point multiply failed:\n");
      print_mpi ("r", a);
      print_mpi ("Rx", x);
      print_mpi ("Ry", y);
    }


  gcry_mpi_release (I);
  gcry_mpi_release (b);
  gcry_mpi_release (n);
  gcry_mpi_release (p);
  gcry_mpi_release (w);
  gcry_mpi_release (a);
  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);
  gcry_mpi_point_release (Q);
  gcry_mpi_point_release (G);
  gcry_mpi_release (k);
  gcry_ctx_release (ctx);
}


int
main (int argc, char **argv)
{

  if (argc > 1 && !strcmp (argv[1], "--verbose"))
    verbose = 1;
  else if (argc > 1 && !strcmp (argv[1], "--debug"))
    verbose = debug = 1;

  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");

  xgcry_control (GCRYCTL_DISABLE_SECMEM, 0);
  xgcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);
  if (debug)
    xgcry_control (GCRYCTL_SET_DEBUG_FLAGS, 1u, 0);
  xgcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);

  set_get_point ();
  context_alloc ();
  context_param ();
  basic_ec_math ();

  /* The tests are for P-192 and ed25519 which are not supported in
     FIPS mode.  */
  if (!gcry_fips_mode_active())
    {
      basic_ec_math_simplified ();
      twistededwards_math ();
    }

  info ("All tests completed. Errors: %d\n", error_count);
  return error_count ? 1 : 0;
}
