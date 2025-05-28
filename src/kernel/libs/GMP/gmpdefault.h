#pragma once

#include <limits.h>
#include "defaultInclude.h"
#include "assert.h"

#ifndef __GNU_MP__
#define __GNU_MP__ 6

/* Instantiated by configure. */
#if !defined(__GMP_WITHIN_CONFIGURE)
/* #undef _LONG_LONG_LIMB */
#define __GMP_LIBGMP_DLL 0
#endif

/* __GMP_DECLSPEC supports Windows DLL versions of libgmp, and is empty in
   all other circumstances.

   When compiling objects for libgmp, __GMP_DECLSPEC is an export directive,
   or when compiling for an application it's an import directive.  The two
   cases are differentiated by __GMP_WITHIN_GMP defined by the GMP Makefiles
   (and not defined from an application).

   __GMP_DECLSPEC_XX is similarly used for libgmpxx.  __GMP_WITHIN_GMPXX
   indicates when building libgmpxx, and in that case libgmpxx functions are
   exports, but libgmp functions which might get called are imports.

   Libtool DLL_EXPORT define is not used.

   There's no attempt to support GMP built both static and DLL.  Doing so
   would mean applications would have to tell us which of the two is going
   to be used when linking, and that seems very tedious and error prone if
   using GMP by hand, and equally tedious from a package since autoconf and
   automake don't give much help.

   __GMP_DECLSPEC is required on all documented global functions and
   variables, the various internals in gmp-impl.h etc can be left unadorned.
   But internals used by the test programs or speed measuring programs
   should have __GMP_DECLSPEC, and certainly constants or variables must
   have it or the wrong address will be resolved.

   In gcc __declspec can go at either the start or end of a prototype.

   In Microsoft C __declspec must go at the start, or after the type like
   void __declspec(...) *foo()".  There's no __dllexport or anything to
   guard against someone foolish #defining dllexport.  _export used to be
   available, but no longer.

   In Borland C _export still exists, but needs to go after the type, like
   "void _export foo();".  Would have to change the __GMP_DECLSPEC syntax to
   make use of that.  Probably more trouble than it's worth.  */

#if defined(__GNUC__)
#define __GMP_DECLSPEC_EXPORT __declspec(__dllexport__)
#define __GMP_DECLSPEC_IMPORT __declspec(__dllimport__)
#endif
#if defined(_MSC_VER) || defined(__BORLANDC__)
#define __GMP_DECLSPEC_EXPORT __declspec(dllexport)
#define __GMP_DECLSPEC_IMPORT __declspec(dllimport)
#endif
#ifdef __WATCOMC__
#define __GMP_DECLSPEC_EXPORT __export
#define __GMP_DECLSPEC_IMPORT __import
#endif
#ifdef __IBMC__
#define __GMP_DECLSPEC_EXPORT _Export
#define __GMP_DECLSPEC_IMPORT _Import
#endif

#if __GMP_LIBGMP_DLL
#ifdef __GMP_WITHIN_GMP
/* compiling to go into a DLL libgmp */
#define __GMP_DECLSPEC __GMP_DECLSPEC_EXPORT
#else
/* compiling to go into an application which will link to a DLL libgmp */
#define __GMP_DECLSPEC __GMP_DECLSPEC_IMPORT
#endif
#else
/* all other cases */
#define __GMP_DECLSPEC
#endif

typedef uint32_t mp_limb_t;
typedef int32_t mp_limb_signed_t;
typedef uint32_t mp_bitcnt_t;

/* For reference, note that the name mpz_struct gets into C++ mangled
   function names, which means although the "__" suggests an internal, we
   must leave this name for binary compatibility.  */
typedef struct mpz_struct_t
{
    int _mp_alloc;    /* Number of *limbs* allocated and pointed
                to by the _mp_d field.  */
    int _mp_size;     /* abs(_mp_size) is the number of limbs the
             last field points to.  If _mp_size is
             negative this is a negative number.  */
    mp_limb_t *_mp_d; /* Pointer to the limbs.  */
} mpz_struct;

#endif // __GNU_MP__

typedef mpz_struct MP_INT; /* gmp 1 source compatibility */
typedef mpz_struct mpz_t[1];

typedef mp_limb_t *mp_ptr;
typedef const mp_limb_t *mp_srcptr;

#define __GMP_MP_SIZE_T_INT 0
typedef long int mp_size_t;
typedef long int mp_exp_t;

/* ==== mpq ==== */

typedef struct
{
    mpz_struct _mp_num;
    mpz_struct _mp_den;
} __mpq_struct;

typedef __mpq_struct MP_RAT; /* gmp 1 source compatibility */
typedef __mpq_struct mpq_t[1];

/* ==== mpf ==== */
typedef struct
{
    int _mp_prec;     /* Max precision, in number of `mp_limb_t's.
             Set by mpf_init and modified by
             mpf_set_prec.  The area pointed to by the
             _mp_d field contains `prec' + 1 limbs.  */
    int _mp_size;     /* abs(_mp_size) is the number of limbs the
             last field points to.  If _mp_size is
             negative this is a negative number.  */
    mp_exp_t _mp_exp; /* Exponent, in the base of `mp_limb_t'.  */
    mp_limb_t *_mp_d; /* Pointer to the limbs.  */
} __mpf_struct;

/* typedef __mpf_struct MP_FLOAT; */
typedef __mpf_struct* mpf_t;

/* Available random number generation algorithms.  */
typedef enum
{
    GMP_RAND_ALG_DEFAULT = 0,
    GMP_RAND_ALG_LC = GMP_RAND_ALG_DEFAULT /* Linear congruential.  */
} gmp_randalg_t;

/* Random state struct.  */
typedef struct
{
    mpz_struct *_mp_seed;  /* _mp_d member points to state of the generator. */
    gmp_randalg_t _mp_alg; /* Currently unused. */
    union
    {
        void *_mp_lc; /* Pointer to function pointers structure.  */
    } _mp_algdata;
} __gmp_randstate_struct;
typedef __gmp_randstate_struct* gmp_randstate_t;

/* Types for function declarations in gmp files.  */
/* ??? Should not pollute user name space with these ??? */
typedef const mpz_struct *mpz_srcptr;
typedef mpz_struct *mpz_ptr;
typedef const __mpf_struct *mpf_srcptr;
typedef __mpf_struct *mpf_ptr;
typedef const __mpq_struct *mpq_srcptr;
typedef __mpq_struct *mpq_ptr;

#if __GMP_LIBGMP_DLL
#ifdef __GMP_WITHIN_GMPXX
/* compiling to go into a DLL libgmpxx */
#define __GMP_DECLSPEC_XX __GMP_DECLSPEC_EXPORT
#else
/* compiling to go into a application which will link to a DLL libgmpxx */
#define __GMP_DECLSPEC_XX __GMP_DECLSPEC_IMPORT
#endif
#else
/* all other cases */
#define __GMP_DECLSPEC_XX
#endif

#ifndef __MPN
#define __MPN(x) __gmpn_##x
#endif

/* For reference, "defined(EOF)" cannot be used here.  In g++ 2.95.4,
   <iostream> defines EOF but not FILE.  */
#if defined(FILE) || defined(H_STDIO) || defined(_H_STDIO) /* AIX */             \
    || defined(_STDIO_H)                                   /* glibc, Sun, SCO */ \
    || defined(_STDIO_H_)                                  /* BSD, OSF */        \
    || defined(__STDIO_H)                                  /* Borland */         \
    || defined(__STDIO_H__)                                /* IRIX */            \
    || defined(_STDIO_INCLUDED)                            /* HPUX */            \
    || defined(__dj_include_stdio_h_)                      /* DJGPP */           \
    || defined(_FILE_DEFINED)                              /* Microsoft */       \
    || defined(__STDIO__)                                  /* Apple MPW MrC */   \
    || defined(_MSL_STDIO_H)                               /* Metrowerks */      \
    || defined(_STDIO_H_INCLUDED)                          /* QNX4 */            \
    || defined(_ISO_STDIO_ISO_H)                           /* Sun C++ */         \
    || defined(__STDIO_LOADED)                             /* VMS */             \
    || defined(__DEFINED_FILE)                             /* musl */
#define _GMP_H_HAVE_FILE 1
#endif

/* In ISO C, if a prototype involving "struct obstack *" is given without
   that structure defined, then the struct is scoped down to just the
   prototype, causing a conflict if it's subsequently defined for real.  So
   only give prototypes if we've got obstack.h.  */
#if defined(_OBSTACK_H) /* glibc <obstack.h> */
#define _GMP_H_HAVE_OBSTACK 1
#endif

/* The prototypes for gmp_vprintf etc are provided only if va_list is defined,
   via an application having included <stdarg.h>.  Usually va_list is a typedef
   so can't be tested directly, but C99 specifies that va_start is a macro.

   <stdio.h> will define some sort of va_list for vprintf and vfprintf, but
   let's not bother trying to use that since it's not standard and since
   application uses for gmp_vprintf etc will almost certainly require the
   whole <stdarg.h> anyway.  */

#ifdef va_start
#define _GMP_H_HAVE_VA_LIST 1
#endif

/* Test for gcc >= maj.min, as per __GNUC_PREREQ in glibc */
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define __GMP_GNUC_PREREQ(maj, min) \
    ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define __GMP_GNUC_PREREQ(maj, min) 0
#endif

/* "pure" is in gcc 2.96 and up, see "(gcc)Function Attributes".  Basically
   it means a function does nothing but examine its arguments and memory
   (global or via arguments) to generate a return value, but changes nothing
   and has no side-effects.  __GMP_NO_ATTRIBUTE_CONST_PURE lets
   tune/common.c etc turn this off when trying to write timing loops.  */
#if __GMP_GNUC_PREREQ(2, 96) && !defined(__GMP_NO_ATTRIBUTE_CONST_PURE)
#define __GMP_ATTRIBUTE_PURE __attribute__((__pure__))
#else
#define __GMP_ATTRIBUTE_PURE
#endif

/* __GMP_CAST allows us to use static_cast in C++, so our macros are clean
   to "g++ -Wold-style-cast".

   Casts in "extern inline" code within an extern "C" block don't induce
   these warnings, so __GMP_CAST only needs to be used on documented
   macros.  */

#define __GMP_CAST(type, expr) ((type)(expr))

/* PORTME: What other compilers have a useful "extern inline"?  "static
   inline" would be an acceptable substitute if the compiler (or linker)
   discards unused statics.  */

/* gcc has __inline__ in all modes, including strict ansi.  Give a prototype
   for an inline too, so as to correctly specify "dllimport" on windows, in
   case the function is called rather than inlined.
   GCC 4.3 and above with -std=c99 or -std=gnu99 implements ISO C99
   inline semantics, unless -fgnu89-inline is used.  */
#ifdef __GNUC__
#if (defined __GNUC_STDC_INLINE__) || (__GNUC__ == 4 && __GNUC_MINOR__ == 2) || (defined __GNUC_GNU_INLINE__ && defined __cplusplus)
#define __GMP_EXTERN_INLINE extern __inline__ __attribute__((__gnu_inline__))
#else
#define __GMP_EXTERN_INLINE extern __inline__
#endif
#define __GMP_INLINE_PROTOTYPES 1
#endif

/* By default, don't give a prototype when there's going to be an inline
   version.  Note in particular that Cray C++ objects to the combination of
   prototype and inline.  */
#ifdef __GMP_EXTERN_INLINE
#ifndef __GMP_INLINE_PROTOTYPES
#define __GMP_INLINE_PROTOTYPES 0
#endif
#else
#define __GMP_INLINE_PROTOTYPES 1
#endif

#define __GMP_ABS(x) ((x) >= 0 ? (x) : -(x))
#define __GMP_MAX(h, i) ((h) > (i) ? (h) : (i))

#ifndef __builtin_expect
#define __builtin_expect(a, b) (a)
#endif

/* __builtin_expect is in gcc 3.0, and not in 2.95. */
#if __GMP_GNUC_PREREQ(3, 0)
#define __GMP_LIKELY(cond) __builtin_expect((cond) != 0, 1)
#define __GMP_UNLIKELY(cond) __builtin_expect((cond) != 0, 0)
#else
#define __GMP_LIKELY(cond) (cond)
#define __GMP_UNLIKELY(cond) (cond)
#endif

#define __GMP_CRAY_Pragma(str)

/* Allow direct user access to numerator and denominator of an mpq_t object.  */
#define mpq_numref(Q) (&((Q)->_mp_num))
#define mpq_denref(Q) (&((Q)->_mp_den))

#define GMP_LIMB_BITS (4 * CHAR_BIT)
#define GMP_LIMB_BYTES sizeof(GMP_ULONG_BITS)
#define BITS_PER_ULONG (8 * 4)

#define GMP_LIMB_MAX ((mp_limb_t) ~(mp_limb_t)0)
#define GMP_LIMB_HIGHBIT ((mp_limb_t)1 << (GMP_LIMB_BITS - 1))

#define GMP_HLIMB_BIT ((mp_limb_t)1 << (GMP_LIMB_BITS / 2))
#define GMP_LLIMB_MASK (GMP_HLIMB_BIT - 1)

#define GMP_ULONG_BITS (sizeof(unsigned long) * CHAR_BIT)
#define GMP_ULONG_HIGHBIT ((unsigned long)1 << (GMP_ULONG_BITS - 1))

#define GMP_ABS(x) ((x) >= 0 ? (x) : -(x))
#define GMP_NEG_CAST(T, x) (-((T)((x) + 1) - 1))

#define GMP_MIN(a, b) ((a) < (b) ? (a) : (b))
#define GMP_MAX(a, b) ((a) > (b) ? (a) : (b))

#define GMP_CMP(a, b) (((a) > (b)) - ((a) < (b)))

#if defined(DBL_MANT_DIG) && FLT_RADIX == 2
#define GMP_DBL_MANT_BITS DBL_MANT_DIG
#else
#define GMP_DBL_MANT_BITS (53)
#endif

int gmp_mpn_overlap_p(mp_ptr xp, mp_size_t xsize, mp_ptr yp, mp_size_t ysize);
void gmp_assert_nocarry(mp_limb_t x);

void gmp_clz(unsigned *count, mp_limb_t x);

void gmp_ctz(unsigned *count, mp_limb_t x);

void gmp_add_ssaaaa(mp_limb_t *sh, mp_limb_t *sl, mp_limb_t ah, mp_limb_t al, mp_limb_t bh, mp_limb_t bl);

void gmp_sub_ddmmss(mp_limb_t *sh, mp_limb_t *sl, mp_limb_t ah, mp_limb_t al, mp_limb_t bh, mp_limb_t bl);

void gmp_umul_ppmm(mp_limb_t *w1, mp_limb_t *w0, mp_limb_t u, mp_limb_t v);

void gmp_udiv_qrnnd_preinv(mp_limb_t *q, mp_limb_t *r, mp_limb_t nh, mp_limb_t nl, mp_limb_t d, mp_limb_t di);

void gmp_udiv_qr_3by2(mp_limb_t *q, mp_limb_t *r1, mp_limb_t *r0, mp_limb_t n2, mp_limb_t n1, mp_limb_t n0, mp_limb_t d1, mp_limb_t d0, mp_limb_t dinv);

/* Swap macros. */
void mp_limb_t_swap(mp_limb_t *x, mp_limb_t *y);
void mp_size_t_swap(mp_size_t *x, mp_size_t *y);
void mp_bitcnt_t_swap(mp_bitcnt_t *x, mp_bitcnt_t *y);
void mp_ptr_swap(mp_ptr *x, mp_ptr *y);
void mp_srcptr_swap(mp_srcptr *x, mp_srcptr *y);

#define MPN_PTR_SWAP(xp, xs, yp, ys) \
    do                               \
    {                                \
        MP_PTR_SWAP(xp, yp);         \
        MP_SIZE_T_SWAP(xs, ys);      \
    } while (0)
#define MPN_SRCPTR_SWAP(xp, xs, yp, ys) \
    do                                  \
    {                                   \
        MP_SRCPTR_SWAP(xp, yp);         \
        MP_SIZE_T_SWAP(xs, ys);         \
    } while (0)

#define MPZ_PTR_SWAP(x, y)                 \
    do                                     \
    {                                      \
        mpz_ptr __mpz_ptr_swap__tmp = (x); \
        (x) = (y);                         \
        (y) = __mpz_ptr_swap__tmp;         \
    } while (0)
#define MPZ_SRCPTR_SWAP(x, y)                    \
    do                                           \
    {                                            \
        mpz_srcptr __mpz_srcptr_swap__tmp = (x); \
        (x) = (y);                               \
        (y) = __mpz_srcptr_swap__tmp;            \
    } while (0)

extern int mp_bits_per_limb;

/* Dummy for non-gcc, code involving it will go dead. */
#if !__GNUC__ && __GNUC__ < 2
// #define __builtin_constant_p(x) 0
#endif

typedef struct mpn_base_info_t
{
  /* bb is the largest power of the base which fits in one limb, and
     exp is the corresponding exponent. */
  unsigned exp;
  mp_limb_t bb;
} mpn_base_info;
