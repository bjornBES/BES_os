/* This file generated by gen-fib.c - DO NOT EDIT. */

#include "libs/GMP/defaultincs.h"

#if GMP_NUMB_BITS != 32
Error, error, this data is for 64 bits
#endif

const mp_limb_t
__gmp_fib_table[FIB_TABLE_LIMIT+2] = {
  CNST_LIMB (0x1),  /* -1 */
  CNST_LIMB (0x0),  /* 0 */
  CNST_LIMB (0x1),  /* 1 */
  CNST_LIMB (0x1),  /* 2 */
  CNST_LIMB (0x2),  /* 3 */
  CNST_LIMB (0x3),  /* 4 */
  CNST_LIMB (0x5),  /* 5 */
  CNST_LIMB (0x8),  /* 6 */
  CNST_LIMB (0xd),  /* 7 */
  CNST_LIMB (0x15),  /* 8 */
  CNST_LIMB (0x22),  /* 9 */
  CNST_LIMB (0x37),  /* 10 */
  CNST_LIMB (0x59),  /* 11 */
  CNST_LIMB (0x90),  /* 12 */
  CNST_LIMB (0xe9),  /* 13 */
  CNST_LIMB (0x179),  /* 14 */
  CNST_LIMB (0x262),  /* 15 */
  CNST_LIMB (0x3db),  /* 16 */
  CNST_LIMB (0x63d),  /* 17 */
  CNST_LIMB (0xa18),  /* 18 */
  CNST_LIMB (0x1055),  /* 19 */
  CNST_LIMB (0x1a6d),  /* 20 */
  CNST_LIMB (0x2ac2),  /* 21 */
  CNST_LIMB (0x452f),  /* 22 */
  CNST_LIMB (0x6ff1),  /* 23 */
  CNST_LIMB (0xb520),  /* 24 */
  CNST_LIMB (0x12511),  /* 25 */
  CNST_LIMB (0x1da31),  /* 26 */
  CNST_LIMB (0x2ff42),  /* 27 */
  CNST_LIMB (0x4d973),  /* 28 */
  CNST_LIMB (0x7d8b5),  /* 29 */
  CNST_LIMB (0xcb228),  /* 30 */
  CNST_LIMB (0x148add),  /* 31 */
  CNST_LIMB (0x213d05),  /* 32 */
  CNST_LIMB (0x35c7e2),  /* 33 */
  CNST_LIMB (0x5704e7),  /* 34 */
  CNST_LIMB (0x8cccc9),  /* 35 */
  CNST_LIMB (0xe3d1b0),  /* 36 */
  CNST_LIMB (0x1709e79),  /* 37 */
  CNST_LIMB (0x2547029),  /* 38 */
  CNST_LIMB (0x3c50ea2),  /* 39 */
  CNST_LIMB (0x6197ecb),  /* 40 */
  CNST_LIMB (0x9de8d6d),  /* 41 */
  CNST_LIMB (0xff80c38),  /* 42 */
  CNST_LIMB (0x19d699a5),  /* 43 */
  CNST_LIMB (0x29cea5dd),  /* 44 */
  CNST_LIMB (0x43a53f82),  /* 45 */
  CNST_LIMB (0x6d73e55f),  /* 46 */
  CNST_LIMB (0xb11924e1),  /* 47 */
  CNST_LIMB (0x11e8d0a40),  /* 48 */
  CNST_LIMB (0x1cfa62f21),  /* 49 */
  CNST_LIMB (0x2ee333961),  /* 50 */
  CNST_LIMB (0x4bdd96882),  /* 51 */
  CNST_LIMB (0x7ac0ca1e3),  /* 52 */
  CNST_LIMB (0xc69e60a65),  /* 53 */
  CNST_LIMB (0x1415f2ac48),  /* 54 */
  CNST_LIMB (0x207fd8b6ad),  /* 55 */
  CNST_LIMB (0x3495cb62f5),  /* 56 */
  CNST_LIMB (0x5515a419a2),  /* 57 */
  CNST_LIMB (0x89ab6f7c97),  /* 58 */
  CNST_LIMB (0xdec1139639),  /* 59 */
  CNST_LIMB (0x1686c8312d0),  /* 60 */
  CNST_LIMB (0x2472d96a909),  /* 61 */
  CNST_LIMB (0x3af9a19bbd9),  /* 62 */
  CNST_LIMB (0x5f6c7b064e2),  /* 63 */
  CNST_LIMB (0x9a661ca20bb),  /* 64 */
  CNST_LIMB (0xf9d297a859d),  /* 65 */
  CNST_LIMB (0x19438b44a658),  /* 66 */
  CNST_LIMB (0x28e0b4bf2bf5),  /* 67 */
  CNST_LIMB (0x42244003d24d),  /* 68 */
  CNST_LIMB (0x6b04f4c2fe42),  /* 69 */
  CNST_LIMB (0xad2934c6d08f),  /* 70 */
  CNST_LIMB (0x1182e2989ced1),  /* 71 */
  CNST_LIMB (0x1c5575e509f60),  /* 72 */
  CNST_LIMB (0x2dd8587da6e31),  /* 73 */
  CNST_LIMB (0x4a2dce62b0d91),  /* 74 */
  CNST_LIMB (0x780626e057bc2),  /* 75 */
  CNST_LIMB (0xc233f54308953),  /* 76 */
  CNST_LIMB (0x13a3a1c2360515),  /* 77 */
  CNST_LIMB (0x1fc6e116668e68),  /* 78 */
  CNST_LIMB (0x336a82d89c937d),  /* 79 */
  CNST_LIMB (0x533163ef0321e5),  /* 80 */
  CNST_LIMB (0x869be6c79fb562),  /* 81 */
  CNST_LIMB (0xd9cd4ab6a2d747),  /* 82 */
  CNST_LIMB (0x16069317e428ca9),  /* 83 */
  CNST_LIMB (0x23a367c34e563f0),  /* 84 */
  CNST_LIMB (0x39a9fadb327f099),  /* 85 */
  CNST_LIMB (0x5d4d629e80d5489),  /* 86 */
  CNST_LIMB (0x96f75d79b354522),  /* 87 */
  CNST_LIMB (0xf444c01834299ab),  /* 88 */
  CNST_LIMB (0x18b3c1d91e77decd),  /* 89 */
  CNST_LIMB (0x27f80ddaa1ba7878),  /* 90 */
  CNST_LIMB (0x40abcfb3c0325745),  /* 91 */
  CNST_LIMB (0x68a3dd8e61eccfbd),  /* 92 */
  CNST_LIMB (0xa94fad42221f2702),  /* 93 */
};
