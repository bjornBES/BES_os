#pragma once

#ifndef __GMP_H__

/* Instantiated by configure. */
#if !defined(__GMP_WITHIN_CONFIGURE)
#define __GMP_HAVE_HOST_CPU_FAMILY_power 0
#define __GMP_HAVE_HOST_CPU_FAMILY_powerpc 0
#define GMP_NAIL_BITS 0
#endif
#define GMP_NUMB_BITS (GMP_LIMB_BITS - GMP_NAIL_BITS)
#define GMP_NUMB_MASK ((~__GMP_CAST(mp_limb_t, 0)) >> GMP_NAIL_BITS)
#define GMP_NUMB_MAX GMP_NUMB_MASK
#define GMP_NAIL_MASK (~GMP_NUMB_MASK)

#include "gmpdefault.h"

#define mp_set_memory_functions __gmp_set_memory_functions
void mp_set_memory_functions(void *(*)(size_t),
                             void *(*)(void *, size_t, size_t),
                             void (*)(void *, size_t));

#define mp_get_memory_functions __gmp_get_memory_functions
void mp_get_memory_functions(void *(**)(size_t),
                             void *(**)(void *, size_t, size_t),
                             void (**)(void *, size_t));

#define mp_bits_per_limb __gmp_bits_per_limb
extern const int mp_bits_per_limb;

#define gmp_errno __gmp_errno
extern int gmp_errno;

#define gmp_version __gmp_version
extern const char *const gmp_version;

/**************** Random number routines.  ****************/

/* obsolete */
#define gmp_randinit __gmp_randinit
void gmp_randinit(gmp_randstate_t, gmp_randalg_t, ...);

#define gmp_randinit_default __gmp_randinit_default
void gmp_randinit_default(gmp_randstate_t);

#define gmp_randinit_lc_2exp __gmp_randinit_lc_2exp
void gmp_randinit_lc_2exp(gmp_randstate_t, mpz_srcptr, unsigned long int, mp_bitcnt_t);

#define gmp_randinit_lc_2exp_size __gmp_randinit_lc_2exp_size
int gmp_randinit_lc_2exp_size(gmp_randstate_t, mp_bitcnt_t);

#define gmp_randinit_mt __gmp_randinit_mt
void gmp_randinit_mt(gmp_randstate_t);

#define gmp_randinit_set __gmp_randinit_set
void gmp_randinit_set(gmp_randstate_t, const __gmp_randstate_struct *);

#define gmp_randseed __gmp_randseed
void gmp_randseed(gmp_randstate_t, mpz_srcptr);

#define gmp_randseed_ui __gmp_randseed_ui
void gmp_randseed_ui(gmp_randstate_t, unsigned long int);

#define gmp_randclear __gmp_randclear
void gmp_randclear(gmp_randstate_t);

#define gmp_urandomb_ui __gmp_urandomb_ui
unsigned long gmp_urandomb_ui(gmp_randstate_t, unsigned long);

#define gmp_urandomm_ui __gmp_urandomm_ui
unsigned long gmp_urandomm_ui(gmp_randstate_t, unsigned long);

/**************** Formatted output routines.  ****************/

#define gmp_asprintf __gmp_asprintf
int gmp_asprintf(char **, const char *, ...);

#define gmp_fprintf __gmp_fprintf
#ifdef _GMP_H_HAVE_FILE
int gmp_fprintf(FILE *, const char *, ...);
#endif

#define gmp_obstack_printf __gmp_obstack_printf
#if defined(_GMP_H_HAVE_OBSTACK)
int gmp_obstack_printf(struct obstack *, const char *, ...);
#endif

#define gmp_obstack_vprintf __gmp_obstack_vprintf
#if defined(_GMP_H_HAVE_OBSTACK) && defined(_GMP_H_HAVE_VA_LIST)
int gmp_obstack_vprintf(struct obstack *, const char *, va_list);
#endif

#define gmp_printf __gmp_printf
int gmp_printf(const char *, ...);

#define gmp_snprintf __gmp_snprintf
int gmp_snprintf(char *, size_t, const char *, ...);

#define gmp_sprintf __gmp_sprintf
int gmp_sprintf(char *, const char *, ...);

#define gmp_vasprintf __gmp_vasprintf
#if defined(_GMP_H_HAVE_VA_LIST)
int gmp_vasprintf(char **, const char *, va_list);
#endif

#define gmp_vfprintf __gmp_vfprintf
#if defined(_GMP_H_HAVE_FILE) && defined(_GMP_H_HAVE_VA_LIST)
int gmp_vfprintf(FILE *, const char *, va_list);
#endif

#define gmp_vprintf __gmp_vprintf
#if defined(_GMP_H_HAVE_VA_LIST)
int gmp_vprintf(const char *, va_list);
#endif

#define gmp_vsnprintf __gmp_vsnprintf
#if defined(_GMP_H_HAVE_VA_LIST)
int gmp_vsnprintf(char *, size_t, const char *, va_list);
#endif

#define gmp_vsprintf __gmp_vsprintf
#if defined(_GMP_H_HAVE_VA_LIST)
int gmp_vsprintf(char *, const char *, va_list);
#endif

/**************** Formatted input routines.  ****************/

#define gmp_fscanf __gmp_fscanf
#ifdef _GMP_H_HAVE_FILE
int gmp_fscanf(FILE *, const char *, ...);
#endif

#define gmp_scanf __gmp_scanf
int gmp_scanf(const char *, ...);

#define gmp_sscanf __gmp_sscanf
int gmp_sscanf(const char *, const char *, ...);

#define gmp_vfscanf __gmp_vfscanf
#if defined(_GMP_H_HAVE_FILE) && defined(_GMP_H_HAVE_VA_LIST)
int gmp_vfscanf(FILE *, const char *, va_list);
#endif

#define gmp_vscanf __gmp_vscanf
#if defined(_GMP_H_HAVE_VA_LIST)
int gmp_vscanf(const char *, va_list);
#endif

#define gmp_vsscanf __gmp_vsscanf
#if defined(_GMP_H_HAVE_VA_LIST)
int gmp_vsscanf(const char *, const char *, va_list);
#endif

/**************** Integer (i.e. Z) routines.  ****************/

#define _mpz_realloc __gmpz_realloc
void *_mpz_realloc(mpz_ptr, mp_size_t);

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpz_abs)
void mpz_abs(mpz_ptr, mpz_srcptr);
#endif

void mpz_add(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_add_ui(mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_addmul(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_addmul_ui(mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_and(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_array_init(mpz_ptr, mp_size_t, mp_size_t);

void mpz_bin_ui(mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_bin_uiui(mpz_ptr, unsigned long int, unsigned long int);

void mpz_cdiv_q(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_cdiv_q_2exp(mpz_ptr, mpz_srcptr, mp_bitcnt_t);

unsigned long int mpz_cdiv_q_ui(mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_cdiv_qr(mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);

unsigned long int mpz_cdiv_qr_ui(mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_cdiv_r(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_cdiv_r_2exp(mpz_ptr, mpz_srcptr, mp_bitcnt_t);

unsigned long int mpz_cdiv_r_ui(mpz_ptr, mpz_srcptr, unsigned long int);

unsigned long int mpz_cdiv_ui(mpz_srcptr, unsigned long int);

void mpz_clear(mpz_ptr);

void mpz_clears(mpz_ptr, ...);

void mpz_clrbit(mpz_ptr, mp_bitcnt_t);

int mpz_cmp(mpz_srcptr, mpz_srcptr);

int mpz_cmp_d(mpz_srcptr, double);

int mpz_sgn(mpz_ptr);

#define _mpz_cmp_si __gmpz_cmp_si
int _mpz_cmp_si(mpz_ptr, signed long int);

#define _mpz_cmp_ui __gmpz_cmp_ui
int _mpz_cmp_ui(mpz_ptr, unsigned long int);

int mpz_cmpabs(mpz_srcptr, mpz_srcptr);

int mpz_cmpabs_d(mpz_srcptr, double);

int mpz_cmpabs_ui(mpz_srcptr, unsigned long int);

void mpz_com(mpz_ptr, mpz_srcptr);

void mpz_combit(mpz_ptr, mp_bitcnt_t);

int mpz_congruent_p(mpz_srcptr, mpz_srcptr, mpz_srcptr);

int mpz_congruent_2exp_p(mpz_srcptr, mpz_srcptr, mp_bitcnt_t);

int mpz_congruent_ui_p(mpz_srcptr, unsigned long, unsigned long);

void mpz_divexact(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_divexact_ui(mpz_ptr, mpz_srcptr, unsigned long);

int mpz_divisible_p(mpz_srcptr, mpz_srcptr);

int mpz_divisible_ui_p(mpz_srcptr, unsigned long);

int mpz_divisible_2exp_p(mpz_srcptr, mp_bitcnt_t);

void mpz_dump(mpz_srcptr);

void *mpz_export(void *, size_t *, int, size_t, int, size_t, mpz_srcptr);

void mpz_fac_ui(mpz_ptr, unsigned long int);

void mpz_2fac_ui(mpz_ptr, unsigned long int);

void mpz_mfac_uiui(mpz_ptr, unsigned long int, unsigned long int);

void mpz_primorial_ui(mpz_ptr, unsigned long int);

void mpz_fdiv_q(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_fdiv_q_2exp(mpz_ptr, mpz_srcptr, mp_bitcnt_t);

unsigned long int mpz_fdiv_q_ui(mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_fdiv_qr(mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);

unsigned long int mpz_fdiv_qr_ui(mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_fdiv_r(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_fdiv_r_2exp(mpz_ptr, mpz_srcptr, mp_bitcnt_t);

unsigned long int mpz_fdiv_r_ui(mpz_ptr, mpz_srcptr, unsigned long int);

unsigned long int mpz_fdiv_ui(mpz_srcptr, unsigned long int);

void mpz_fib_ui(mpz_ptr, unsigned long int);

void mpz_fib2_ui(mpz_ptr, mpz_ptr, unsigned long int);

int mpz_fits_sint_p(mpz_srcptr);

int mpz_fits_slong_p(mpz_srcptr);

int mpz_fits_sshort_p(mpz_srcptr);

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpz_fits_uint_p)
int mpz_fits_uint_p(mpz_srcptr);
#endif

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpz_fits_ulong_p)
int mpz_fits_ulong_p(mpz_srcptr);
#endif

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpz_fits_ushort_p)
int mpz_fits_ushort_p(mpz_srcptr);
#endif

void mpz_gcd(mpz_ptr, mpz_srcptr, mpz_srcptr);

unsigned long int mpz_gcd_ui(mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_gcdext(mpz_ptr, mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);

double mpz_get_d(mpz_srcptr);

double mpz_get_d_2exp(signed long int *, mpz_srcptr);

/* signed */ long int mpz_get_si(mpz_struct *);

char *mpz_get_str(char *, int, mpz_ptr);

unsigned long int mpz_get_ui(mpz_ptr);

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpz_getlimbn)
mp_limb_t mpz_getlimbn(mpz_srcptr, mp_size_t);
#endif

mp_bitcnt_t mpz_hamdist(mpz_srcptr, mpz_srcptr);

void mpz_import(mpz_ptr, size_t, int, size_t, int, size_t, const void *);

void mpz_init(mpz_ptr);

void mpz_init2(mpz_ptr, mp_bitcnt_t);

void mpz_inits(mpz_ptr, ...);

void mpz_init_set(mpz_ptr, mpz_srcptr);

void mpz_init_set_d(mpz_ptr, double);

void mpz_init_set_si(mpz_ptr, signed long int);

int mpz_init_set_str(mpz_ptr, const char *, int);

void mpz_init_set_ui(mpz_ptr, unsigned long int);

#ifdef _GMP_H_HAVE_FILE
size_t mpz_inp_raw(mpz_ptr, FILE *);
#endif

#ifdef _GMP_H_HAVE_FILE
size_t mpz_inp_str(mpz_ptr, FILE *, int);
#endif

int mpz_invert(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_ior(mpz_ptr, mpz_srcptr, mpz_srcptr);

int mpz_jacobi(mpz_srcptr, mpz_srcptr);

#define mpz_kronecker mpz_jacobi /* alias */

int mpz_kronecker_si(mpz_srcptr, long);

int mpz_kronecker_ui(mpz_srcptr, unsigned long);

int mpz_si_kronecker(long, mpz_srcptr);

int mpz_ui_kronecker(unsigned long, mpz_srcptr);

void mpz_lcm(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_lcm_ui(mpz_ptr, mpz_srcptr, unsigned long);

#define mpz_legendre mpz_jacobi /* alias */

void mpz_lucnum_ui(mpz_ptr, unsigned long int);

void mpz_lucnum2_ui(mpz_ptr, mpz_ptr, unsigned long int);

int mpz_millerrabin(mpz_srcptr, int);

void mpz_mod(mpz_ptr, mpz_srcptr, mpz_srcptr);

#define mpz_mod_ui mpz_fdiv_r_ui /* same as fdiv_r because divisor unsigned */

void mpz_mul(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_mul_2exp(mpz_ptr, mpz_srcptr, mp_bitcnt_t);

void mpz_mul_si(mpz_ptr, mpz_srcptr, long int);

void mpz_mul_ui(mpz_ptr, mpz_srcptr, unsigned long int);

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpz_neg)
void mpz_neg(mpz_ptr, mpz_srcptr);
#endif

void mpz_nextprime(mpz_ptr, mpz_srcptr);

#ifdef _GMP_H_HAVE_FILE
size_t mpz_out_raw(FILE *, mpz_srcptr);
#endif

#ifdef _GMP_H_HAVE_FILE
size_t mpz_out_str(FILE *, int, mpz_srcptr);
#endif

int mpz_perfect_power_p(mpz_srcptr);

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpz_perfect_square_p)
int mpz_perfect_square_p(mpz_srcptr);
#endif

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpz_popcount)
mp_bitcnt_t mpz_popcount(mpz_srcptr);
#endif

// #define mp\w_[\w]+ __gmp\w_[\w]\n
// #define mpz_[\w]+\s__gmpz_[\w]+
void mpz_pow_ui(mpz_ptr, mpz_ptr, unsigned long int);

void mpz_powm(mpz_ptr, mpz_srcptr, mpz_srcptr, mpz_srcptr);

void mpz_powm_sec(mpz_ptr, mpz_srcptr, mpz_srcptr, mpz_srcptr);

void mpz_powm_ui(mpz_ptr, mpz_srcptr, unsigned long int, mpz_srcptr);

int mpz_probab_prime_p(mpz_srcptr, int);

void mpz_random(mpz_ptr, mp_size_t);

void mpz_random2(mpz_ptr, mp_size_t);

void mpz_realloc2(mpz_ptr, mp_bitcnt_t);

mp_bitcnt_t mpz_remove(mpz_ptr, mpz_srcptr, mpz_srcptr);

int mpz_root(mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_rootrem(mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_rrandomb(mpz_ptr, gmp_randstate_t, mp_bitcnt_t);

mp_bitcnt_t mpz_scan0(mpz_srcptr, mp_bitcnt_t);

mp_bitcnt_t mpz_scan1(mpz_srcptr, mp_bitcnt_t);

void mpz_set(mpz_ptr, mpz_srcptr);

void mpz_set_d(mpz_ptr, double);

void mpz_set_f(mpz_ptr, mpf_srcptr);

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpz_set_q)
void mpz_set_q(mpz_ptr, mpq_srcptr);
#endif

void mpz_set_si(mpz_ptr, signed long int);

int mpz_set_str(mpz_ptr, const char *, int);

void mpz_set_ui(mpz_ptr, unsigned long int);

void mpz_setbit(mpz_ptr, mp_bitcnt_t);

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpz_size)
size_t mpz_size(mpz_srcptr);
#endif

size_t mpz_sizeinbase(mpz_srcptr, int);

void mpz_sqrt(mpz_ptr, mpz_srcptr);

void mpz_sqrtrem(mpz_ptr, mpz_ptr, mpz_srcptr);

void mpz_sub(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_sub_ui(mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_ui_sub(mpz_ptr, unsigned long int, mpz_srcptr);

void mpz_submul(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_submul_ui(mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_swap(mpz_ptr, mpz_ptr);

unsigned long int mpz_tdiv_ui(mpz_srcptr, unsigned long int);

void mpz_tdiv_q(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_tdiv_q_2exp(mpz_ptr, mpz_srcptr, mp_bitcnt_t);

unsigned long int mpz_tdiv_q_ui(mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_tdiv_qr(mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);

unsigned long int mpz_tdiv_qr_ui(mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long int);

void mpz_tdiv_r(mpz_ptr, mpz_srcptr, mpz_srcptr);

void mpz_tdiv_r_2exp(mpz_ptr, mpz_srcptr, mp_bitcnt_t);

unsigned long int mpz_tdiv_r_ui(mpz_ptr, mpz_srcptr, unsigned long int);

int mpz_tstbit(mpz_srcptr, mp_bitcnt_t);

void mpz_ui_pow_ui(mpz_ptr, unsigned long int, mp_size_t);

void mpz_urandomb(mpz_ptr, gmp_randstate_t, mp_bitcnt_t);

void mpz_urandomm(mpz_ptr, gmp_randstate_t, mpz_srcptr);

void mpz_xor(mpz_ptr, mpz_srcptr, mpz_srcptr);

mp_srcptr mpz_limbs_read(mpz_srcptr);

mp_ptr mpz_limbs_write(mpz_ptr, mp_size_t);

mp_ptr mpz_limbs_modify(mpz_ptr, mp_size_t);

void mpz_limbs_finish(mpz_ptr, mp_size_t);

mpz_ptr mpz_roinit_n(mpz_ptr, mp_ptr, mp_size_t);

/**************** Rational (i.e. Q) routines.  ****************/

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpq_abs)
void mpq_abs(mpq_ptr, mpq_srcptr);
#endif

void mpq_add(mpq_ptr, mpq_srcptr, mpq_srcptr);

void mpq_canonicalize(mpq_ptr);

void mpq_clear(mpq_ptr);

void mpq_clears(mpq_ptr, ...);

int mpq_cmp(mpq_srcptr, mpq_srcptr);

#define _mpq_cmp_si __gmpq_cmp_si
int _mpq_cmp_si(mpq_srcptr, long, unsigned long);

#define _mpq_cmp_ui __gmpq_cmp_ui
int _mpq_cmp_ui(mpq_srcptr, unsigned long int, unsigned long int);

int mpq_cmp_z(mpq_srcptr, mpz_srcptr);

void mpq_div(mpq_ptr, mpq_srcptr, mpq_srcptr);

void mpq_div_2exp(mpq_ptr, mpq_srcptr, mp_bitcnt_t);

int mpq_equal(mpq_srcptr, mpq_srcptr);

void mpq_get_num(mpz_ptr, mpq_srcptr);

void mpq_get_den(mpz_ptr, mpq_srcptr);

double mpq_get_d(mpq_srcptr);

char *mpq_get_str(char *, int, mpq_srcptr);

void mpq_init(mpq_ptr);

void mpq_inits(mpq_ptr, ...);

#ifdef _GMP_H_HAVE_FILE
size_t mpq_inp_str(mpq_ptr, FILE *, int);
#endif

void mpq_inv(mpq_ptr, mpq_srcptr);

void mpq_mul(mpq_ptr, mpq_srcptr, mpq_srcptr);

void mpq_mul_2exp(mpq_ptr, mpq_srcptr, mp_bitcnt_t);

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpq_neg)
void mpq_neg(mpq_ptr, mpq_srcptr);
#endif

#ifdef _GMP_H_HAVE_FILE
size_t mpq_out_str(FILE *, int, mpq_srcptr);
#endif

void mpq_set(mpq_ptr, mpq_srcptr);

void mpq_set_d(mpq_ptr, double);

void mpq_set_den(mpq_ptr, mpz_srcptr);

void mpq_set_f(mpq_ptr, mpf_srcptr);

void mpq_set_num(mpq_ptr, mpz_srcptr);

void mpq_set_si(mpq_ptr, signed long int, unsigned long int);

int mpq_set_str(mpq_ptr, const char *, int);

void mpq_set_ui(mpq_ptr, unsigned long int, unsigned long int);

void mpq_set_z(mpq_ptr, mpz_srcptr);

void mpq_sub(mpq_ptr, mpq_srcptr, mpq_srcptr);

void mpq_swap(mpq_ptr, mpq_ptr);

/**************** Float (i.e. F) routines.  ****************/

void mpf_abs(mpf_ptr, mpf_srcptr);

void mpf_add(mpf_ptr, mpf_srcptr, mpf_srcptr);

void mpf_add_ui(mpf_ptr, mpf_srcptr, unsigned long int);
void mpf_ceil(mpf_ptr, mpf_srcptr);

void mpf_clear(mpf_ptr);

void mpf_clears(mpf_ptr, ...);

int mpf_cmp(mpf_srcptr, mpf_srcptr);

int mpf_cmp_z(mpf_srcptr, mpz_srcptr);

int mpf_cmp_d(mpf_srcptr, double);

int mpf_cmp_si(mpf_srcptr, signed long int);

int mpf_cmp_ui(mpf_srcptr, unsigned long int);

void mpf_div(mpf_ptr, mpf_srcptr, mpf_srcptr);

void mpf_div_2exp(mpf_ptr, mpf_srcptr, mp_bitcnt_t);

void mpf_div_ui(mpf_ptr, mpf_srcptr, unsigned long int);

void mpf_dump(mpf_srcptr);

int mpf_eq(mpf_srcptr, mpf_srcptr, mp_bitcnt_t);

int mpf_fits_sint_p(mpf_srcptr);

int mpf_fits_slong_p(mpf_srcptr);

int mpf_fits_sshort_p(mpf_srcptr);

int mpf_fits_uint_p(mpf_srcptr);

int mpf_fits_ulong_p(mpf_srcptr);

int mpf_fits_ushort_p(mpf_srcptr);

void mpf_floor(mpf_ptr, mpf_srcptr);

double mpf_get_d(mpf_srcptr);

double mpf_get_d_2exp(signed long int *, mpf_srcptr);

mp_bitcnt_t mpf_get_default_prec(void);

mp_bitcnt_t mpf_get_prec(mpf_srcptr);

long mpf_get_si(mpf_srcptr);

char *mpf_get_str(char *, mp_exp_t *, int, size_t, mpf_srcptr);

unsigned long mpf_get_ui(mpf_srcptr);

void mpf_init(mpf_ptr);

void mpf_init2(mpf_ptr, mp_bitcnt_t);

void mpf_inits(mpf_ptr, ...);

void mpf_init_set(mpf_ptr, mpf_srcptr);

void mpf_init_set_d(mpf_ptr, double);

void mpf_init_set_si(mpf_ptr, signed long int);

int mpf_init_set_str(mpf_ptr, const char *, int);

void mpf_init_set_ui(mpf_ptr, unsigned long int);

#ifdef _GMP_H_HAVE_FILE
size_t mpf_inp_str(mpf_ptr, FILE *, int);
#endif

int mpf_integer_p(mpf_srcptr);

void mpf_mul(mpf_ptr, mpf_srcptr, mpf_srcptr);

void mpf_mul_2exp(mpf_ptr, mpf_srcptr, mp_bitcnt_t);

void mpf_mul_ui(mpf_ptr, mpf_srcptr, unsigned long int);

void mpf_neg(mpf_ptr, mpf_srcptr);

#ifdef _GMP_H_HAVE_FILE
size_t mpf_out_str(FILE *, int, size_t, mpf_srcptr);
#endif

void mpf_pow_ui(mpf_ptr, mpf_srcptr, unsigned long int);

void mpf_random2(mpf_ptr, mp_size_t, mp_exp_t);

void mpf_reldiff(mpf_ptr, mpf_srcptr, mpf_srcptr);

void mpf_set(mpf_ptr, mpf_srcptr);

void mpf_set_d(mpf_ptr, double);

void mpf_set_default_prec(mp_bitcnt_t);

void mpf_set_prec(mpf_ptr, mp_bitcnt_t);

void mpf_set_prec_raw(mpf_ptr, mp_bitcnt_t);

void mpf_set_q(mpf_ptr, mpq_srcptr);

void mpf_set_si(mpf_ptr, signed long int);

int mpf_set_str(mpf_ptr, const char *, int);

void mpf_set_ui(mpf_ptr, unsigned long int);

void mpf_set_z(mpf_ptr, mpz_srcptr);

size_t mpf_size(mpf_srcptr);

void mpf_sqrt(mpf_ptr, mpf_srcptr);

void mpf_sqrt_ui(mpf_ptr, unsigned long int);

void mpf_sub(mpf_ptr, mpf_srcptr, mpf_srcptr);

void mpf_sub_ui(mpf_ptr, mpf_srcptr, unsigned long int);

void mpf_swap(mpf_ptr, mpf_ptr);

void mpf_trunc(mpf_ptr, mpf_srcptr);

void mpf_ui_div(mpf_ptr, unsigned long int, mpf_srcptr);

void mpf_ui_sub(mpf_ptr, unsigned long int, mpf_srcptr);

void mpf_urandomb(mpf_t, gmp_randstate_t, mp_bitcnt_t);

/************ Low level positive-integer (i.e. N) routines.  ************/

/* This is ugly, but we need to make user calls reach the prefixed function. */

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpn_add)
mp_limb_t mpn_add(mp_ptr, mp_srcptr, mp_size_t, mp_srcptr, mp_size_t);
#endif

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpn_add_1)
mp_limb_t mpn_add_1(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t);
#endif

mp_limb_t mpn_add_n(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);

mp_limb_t mpn_addmul_1(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t);

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpn_cmp)
int mpn_cmp(mp_srcptr, mp_srcptr, mp_size_t);
#endif

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpn_zero_p)
int mpn_zero_p(mp_srcptr, mp_size_t);
#endif

void mpn_divexact_1(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t);

#define mpn_divexact_by3(dst, src, size) \
   mpn_divexact_by3c(dst, src, size, __GMP_CAST(mp_limb_t, 0))

mp_limb_t mpn_divexact_by3c(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t);

#define mpn_divmod_1(qp, np, nsize, dlimb) \
   mpn_divrem_1(qp, __GMP_CAST(mp_size_t, 0), np, nsize, dlimb)

mp_limb_t mpn_divrem(mp_ptr, mp_size_t, mp_ptr, mp_size_t, mp_srcptr, mp_size_t);

mp_limb_t mpn_divrem_1(mp_ptr, mp_size_t, mp_srcptr, mp_size_t, mp_limb_t);

mp_limb_t mpn_divrem_2(mp_ptr, mp_size_t, mp_ptr, mp_size_t, mp_srcptr);

mp_limb_t mpn_div_qr_1(mp_ptr, mp_limb_t *, mp_srcptr, mp_size_t, mp_limb_t);

mp_limb_t mpn_div_qr_2(mp_ptr, mp_ptr, mp_srcptr, mp_size_t, mp_srcptr);

mp_size_t mpn_gcd(mp_ptr, mp_ptr, mp_size_t, mp_ptr, mp_size_t);

mp_limb_t mpn_gcd_11(mp_limb_t, mp_limb_t);

mp_limb_t mpn_gcd_1(mp_srcptr, mp_size_t, mp_limb_t);

mp_limb_t mpn_gcdext_1(mp_limb_signed_t *, mp_limb_signed_t *, mp_limb_t, mp_limb_t);

mp_size_t mpn_gcdext(mp_ptr, mp_ptr, mp_size_t *, mp_ptr, mp_size_t, mp_ptr, mp_size_t);

size_t mpn_get_str(unsigned char *, int, mp_ptr, mp_size_t);

mp_bitcnt_t mpn_hamdist(mp_srcptr, mp_srcptr, mp_size_t);

mp_limb_t mpn_lshift(mp_ptr, mp_srcptr, mp_size_t, unsigned int);

mp_limb_t mpn_mod_1(mp_srcptr, mp_size_t, mp_limb_t);

mp_limb_t mpn_mul(mp_ptr, mp_srcptr, mp_size_t, mp_srcptr, mp_size_t);

mp_limb_t mpn_mul_1(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t);

void mpn_mul_n(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);

void mpn_sqr(mp_ptr, mp_srcptr, mp_size_t);

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpn_neg)
mp_limb_t mpn_neg(mp_ptr, mp_srcptr, mp_size_t);
#endif

void mpn_com(mp_ptr, mp_srcptr, mp_size_t);

int mpn_perfect_square_p(mp_srcptr, mp_size_t);

int mpn_perfect_power_p(mp_srcptr, mp_size_t);

mp_bitcnt_t mpn_popcount(mp_srcptr, mp_size_t);

mp_size_t mpn_pow_1(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t, mp_ptr);

/* undocumented now, but retained here for upward compatibility */
mp_limb_t mpn_preinv_mod_1(mp_srcptr, mp_size_t, mp_limb_t, mp_limb_t);

void mpn_random(mp_ptr, mp_size_t);

void mpn_random2(mp_ptr, mp_size_t);

mp_limb_t mpn_rshift(mp_ptr, mp_srcptr, mp_size_t, unsigned int);

mp_bitcnt_t mpn_scan0(mp_srcptr, mp_bitcnt_t);

mp_bitcnt_t mpn_scan1(mp_srcptr, mp_bitcnt_t);

mp_size_t mpn_set_str(mp_ptr, const unsigned char *, size_t, int);

size_t mpn_sizeinbase(mp_srcptr, mp_size_t, int);

mp_size_t mpn_sqrtrem(mp_ptr, mp_ptr, mp_srcptr, mp_size_t);

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpn_sub)
mp_limb_t mpn_sub(mp_ptr, mp_srcptr, mp_size_t, mp_srcptr, mp_size_t);
#endif

#if __GMP_INLINE_PROTOTYPES || defined(__GMP_FORCE_mpn_sub_1)
mp_limb_t mpn_sub_1(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t);
#endif

mp_limb_t mpn_sub_n(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);

mp_limb_t mpn_submul_1(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t);

void mpn_tdiv_qr(mp_ptr, mp_ptr, mp_size_t, mp_srcptr, mp_size_t, mp_srcptr, mp_size_t);

void mpn_and_n(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
void mpn_andn_n(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
void mpn_nand_n(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
void mpn_ior_n(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
void mpn_iorn_n(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
void mpn_nior_n(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
void mpn_xor_n(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
void mpn_xnor_n(mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);

void mpn_copyi(mp_ptr, mp_srcptr, mp_size_t);
void mpn_copyd(mp_ptr, mp_srcptr, mp_size_t);
void mpn_zero(mp_ptr, mp_size_t);

mp_limb_t mpn_cnd_add_n(mp_limb_t, mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
mp_limb_t mpn_cnd_sub_n(mp_limb_t, mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);

mp_limb_t mpn_sec_add_1(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t, mp_ptr);
mp_size_t mpn_sec_add_1_itch(mp_size_t);

mp_limb_t mpn_sec_sub_1(mp_ptr, mp_srcptr, mp_size_t, mp_limb_t, mp_ptr);
mp_size_t mpn_sec_sub_1_itch(mp_size_t);

void mpn_cnd_swap(mp_limb_t, volatile mp_limb_t *, volatile mp_limb_t *, mp_size_t);

void mpn_sec_mul(mp_ptr, mp_srcptr, mp_size_t, mp_srcptr, mp_size_t, mp_ptr);
mp_size_t mpn_sec_mul_itch(mp_size_t, mp_size_t);

void mpn_sec_sqr(mp_ptr, mp_srcptr, mp_size_t, mp_ptr);
mp_size_t mpn_sec_sqr_itch(mp_size_t);

void mpn_sec_powm(mp_ptr, mp_srcptr, mp_size_t, mp_srcptr, mp_bitcnt_t, mp_srcptr, mp_size_t, mp_ptr);
mp_size_t mpn_sec_powm_itch(mp_size_t, mp_bitcnt_t, mp_size_t);

void mpn_sec_tabselect(volatile mp_limb_t *, volatile const mp_limb_t *, mp_size_t, mp_size_t, mp_size_t);

mp_limb_t mpn_sec_div_qr(mp_ptr, mp_ptr, mp_size_t, mp_srcptr, mp_size_t, mp_ptr);
mp_size_t mpn_sec_div_qr_itch(mp_size_t, mp_size_t);
void mpn_sec_div_r(mp_ptr, mp_size_t, mp_srcptr, mp_size_t, mp_ptr);
mp_size_t mpn_sec_div_r_itch(mp_size_t, mp_size_t);

int mpn_sec_invert(mp_ptr, mp_ptr, mp_srcptr, mp_size_t, mp_bitcnt_t, mp_ptr);
mp_size_t mpn_sec_invert_itch(mp_size_t);

/**************** mpz inlines ****************/

/* The following are provided as inlines where possible, but always exist as
   library functions too, for binary compatibility.

   Within gmp itself this inlining generally isn't relied on, since it
   doesn't get done for all compilers, whereas if something is worth
   inlining then it's worth arranging always.

   There are two styles of inlining here.  When the same bit of code is
   wanted for the inline as for the library version, then __GMP_FORCE_foo
   arranges for that code to be emitted and the __GMP_EXTERN_INLINE
   directive suppressed, eg. mpz_fits_uint_p.  When a different bit of code
   is wanted for the inline than for the library version, then
   __GMP_FORCE_foo arranges the inline to be suppressed, eg. mpz_abs.  */

#if defined(__GMP_EXTERN_INLINE) && !defined(__GMP_FORCE_mpz_abs)
__GMP_EXTERN_INLINE void
mpz_abs(mpz_ptr __gmp_w, mpz_srcptr __gmp_u)
{
   if (__gmp_w != __gmp_u)
      mpz_set(__gmp_w, __gmp_u);
   __gmp_w->_mp_size = __GMP_ABS(__gmp_w->_mp_size);
}
#endif

#if GMP_NAIL_BITS == 0
#define __GMPZ_FITS_UTYPE_P(z, maxval) \
   mp_size_t __gmp_n = z->_mp_size;    \
   mp_ptr __gmp_p = z->_mp_d;          \
   return (__gmp_n == 0 || (__gmp_n == 1 && __gmp_p[0] <= maxval));
#else
#define __GMPZ_FITS_UTYPE_P(z, maxval) \
   mp_size_t __gmp_n = z->_mp_size;    \
   mp_ptr __gmp_p = z->_mp_d;          \
   return (__gmp_n == 0 || (__gmp_n == 1 && __gmp_p[0] <= maxval) || (__gmp_n == 2 && __gmp_p[1] <= ((mp_limb_t)maxval >> GMP_NUMB_BITS)));
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpz_fits_uint_p)
#if !defined(__GMP_FORCE_mpz_fits_uint_p)
__GMP_EXTERN_INLINE
#endif
int mpz_fits_uint_p(mpz_srcptr __gmp_z)
{
   __GMPZ_FITS_UTYPE_P(__gmp_z, UINT_MAX);
}
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpz_fits_ulong_p)
#if !defined(__GMP_FORCE_mpz_fits_ulong_p)
__GMP_EXTERN_INLINE
#endif
int mpz_fits_ulong_p(mpz_srcptr __gmp_z)
{
   __GMPZ_FITS_UTYPE_P(__gmp_z, ULONG_MAX);
}
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpz_fits_ushort_p)
#if !defined(__GMP_FORCE_mpz_fits_ushort_p)
__GMP_EXTERN_INLINE
#endif
int mpz_fits_ushort_p(mpz_srcptr __gmp_z)
{
   __GMPZ_FITS_UTYPE_P(__gmp_z, USHRT_MAX);
}
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpz_getlimbn)
#if !defined(__GMP_FORCE_mpz_getlimbn)
__GMP_EXTERN_INLINE
#endif
mp_limb_t
mpz_getlimbn(mpz_srcptr __gmp_z, mp_size_t __gmp_n)
{
   mp_limb_t __gmp_result = 0;
   if (__GMP_LIKELY(__gmp_n >= 0 && __gmp_n < __GMP_ABS(__gmp_z->_mp_size)))
      __gmp_result = __gmp_z->_mp_d[__gmp_n];
   return __gmp_result;
}
#endif

#if defined(__GMP_EXTERN_INLINE) && !defined(__GMP_FORCE_mpz_neg)
__GMP_EXTERN_INLINE void
mpz_neg(mpz_ptr __gmp_w, mpz_srcptr __gmp_u)
{
   if (__gmp_w != __gmp_u)
      mpz_set(__gmp_w, __gmp_u);
   __gmp_w->_mp_size = -__gmp_w->_mp_size;
}
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpz_perfect_square_p)
#if !defined(__GMP_FORCE_mpz_perfect_square_p)
__GMP_EXTERN_INLINE
#endif
int mpz_perfect_square_p(mpz_srcptr __gmp_a)
{
   mp_size_t __gmp_asize;
   int __gmp_result;

   __gmp_asize = __gmp_a->_mp_size;
   __gmp_result = (__gmp_asize >= 0); /* zero is a square, negatives are not */
   if (__GMP_LIKELY(__gmp_asize > 0))
      __gmp_result = mpn_perfect_square_p(__gmp_a->_mp_d, __gmp_asize);
   return __gmp_result;
}
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpz_popcount)
#if !defined(__GMP_FORCE_mpz_popcount)
__GMP_EXTERN_INLINE
#endif
mp_bitcnt_t
mpz_popcount(mpz_srcptr __gmp_u)
{
   mp_size_t __gmp_usize;
   mp_bitcnt_t __gmp_result;

   __gmp_usize = __gmp_u->_mp_size;
   __gmp_result = (__gmp_usize < 0 ? ~__GMP_CAST(mp_bitcnt_t, 0) : __GMP_CAST(mp_bitcnt_t, 0));
   if (__GMP_LIKELY(__gmp_usize > 0))
      __gmp_result = mpn_popcount(__gmp_u->_mp_d, __gmp_usize);
   return __gmp_result;
}
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpz_set_q)
#if !defined(__GMP_FORCE_mpz_set_q)
__GMP_EXTERN_INLINE
#endif
void mpz_set_q(mpz_ptr __gmp_w, mpq_srcptr __gmp_u)
{
   mpz_tdiv_q(__gmp_w, mpq_numref(__gmp_u), mpq_denref(__gmp_u));
}
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpz_size)
#if !defined(__GMP_FORCE_mpz_size)
__GMP_EXTERN_INLINE
#endif
size_t
mpz_size(mpz_srcptr __gmp_z)
{
   return __GMP_ABS(__gmp_z->_mp_size);
}
#endif

/**************** mpq inlines ****************/

#if defined(__GMP_EXTERN_INLINE) && !defined(__GMP_FORCE_mpq_abs)
__GMP_EXTERN_INLINE void
mpq_abs(mpq_ptr __gmp_w, mpq_srcptr __gmp_u)
{
   if (__gmp_w != __gmp_u)
      mpq_set(__gmp_w, __gmp_u);
   __gmp_w->_mp_num._mp_size = __GMP_ABS(__gmp_w->_mp_num._mp_size);
}
#endif

#if defined(__GMP_EXTERN_INLINE) && !defined(__GMP_FORCE_mpq_neg)
__GMP_EXTERN_INLINE void
mpq_neg(mpq_ptr __gmp_w, mpq_srcptr __gmp_u)
{
   if (__gmp_w != __gmp_u)
      mpq_set(__gmp_w, __gmp_u);
   __gmp_w->_mp_num._mp_size = -__gmp_w->_mp_num._mp_size;
}
#endif

/**************** mpn inlines ****************/

/* The comments with __GMPN_ADD_1 below apply here too.

   The test for FUNCTION returning 0 should predict well.  If it's assumed
   {yp,ysize} will usually have a random number of bits then the high limb
   won't be full and a carry out will occur a good deal less than 50% of the
   time.

   ysize==0 isn't a documented feature, but is used internally in a few
   places.

   Producing cout last stops it using up a register during the main part of
   the calculation, though gcc (as of 3.0) on an "if (mpn_add (...))"
   doesn't seem able to move the true and false legs of the conditional up
   to the two places cout is generated.  */

#define __GMPN_AORS(cout, wp, xp, xsize, yp, ysize, FUNCTION, TEST)  \
   {                                                                 \
      mp_size_t __gmp_i;                                             \
      mp_limb_t __gmp_x;                                             \
                                                                     \
      /* ASSERT ((ysize) >= 0); */                                   \
      /* ASSERT ((xsize) >= (ysize)); */                             \
      /* ASSERT (MPN_SAME_OR_SEPARATE2_P (wp, xsize, xp, xsize)); */ \
      /* ASSERT (MPN_SAME_OR_SEPARATE2_P (wp, xsize, yp, ysize)); */ \
                                                                     \
      __gmp_i = (ysize);                                             \
      if (__gmp_i != 0)                                              \
      {                                                              \
         if (FUNCTION(wp, xp, yp, __gmp_i))                          \
         {                                                           \
            do                                                       \
            {                                                        \
               if (__gmp_i >= (xsize))                               \
               {                                                     \
                  (cout) = 1;                                        \
                  goto __gmp_done;                                   \
               }                                                     \
               __gmp_x = (xp)[__gmp_i];                              \
            } while (TEST);                                          \
         }                                                           \
      }                                                              \
      if ((wp) != (xp))                                              \
         __GMPN_COPY_REST(wp, xp, xsize, __gmp_i);                   \
      (cout) = 0;                                                    \
   __gmp_done:;                                                      \
   }

#define __GMPN_ADD(cout, wp, xp, xsize, yp, ysize)        \
   __GMPN_AORS(cout, wp, xp, xsize, yp, ysize, mpn_add_n, \
               (((wp)[__gmp_i++] = (__gmp_x + 1) & GMP_NUMB_MASK) == 0))
#define __GMPN_SUB(cout, wp, xp, xsize, yp, ysize)        \
   __GMPN_AORS(cout, wp, xp, xsize, yp, ysize, mpn_sub_n, \
               (((wp)[__gmp_i++] = (__gmp_x - 1) & GMP_NUMB_MASK), __gmp_x == 0))

/* The use of __gmp_i indexing is designed to ensure a compile time src==dst
   remains nice and clear to the compiler, so that __GMPN_COPY_REST can
   disappear, and the load/add/store gets a chance to become a
   read-modify-write on CISC CPUs.

   Alternatives:

   Using a pair of pointers instead of indexing would be possible, but gcc
   isn't able to recognise compile-time src==dst in that case, even when the
   pointers are incremented more or less together.  Other compilers would
   very likely have similar difficulty.

   gcc could use "if (__builtin_constant_p(src==dst) && src==dst)" or
   similar to detect a compile-time src==dst.  This works nicely on gcc
   2.95.x, it's not good on gcc 3.0 where __builtin_constant_p(p==p) seems
   to be always false, for a pointer p.  But the current code form seems
   good enough for src==dst anyway.

   gcc on x86 as usual doesn't give particularly good flags handling for the
   carry/borrow detection.  It's tempting to want some multi instruction asm
   blocks to help it, and this was tried, but in truth there's only a few
   instructions to save and any gain is all too easily lost by register
   juggling setting up for the asm.  */

#if GMP_NAIL_BITS == 0
#define __GMPN_AORS_1(cout, dst, src, n, v, OP, CB)        \
   do                                                      \
   {                                                       \
      mp_size_t __gmp_i;                                   \
      mp_limb_t __gmp_x, __gmp_r;                          \
                                                           \
      /* ASSERT ((n) >= 1); */                             \
      /* ASSERT (MPN_SAME_OR_SEPARATE_P (dst, src, n)); */ \
                                                           \
      __gmp_x = (src)[0];                                  \
      __gmp_r = __gmp_x OP(v);                             \
      (dst)[0] = __gmp_r;                                  \
      if (CB(__gmp_r, __gmp_x, (v)))                       \
      {                                                    \
         (cout) = 1;                                       \
         for (__gmp_i = 1; __gmp_i < (n);)                 \
         {                                                 \
            __gmp_x = (src)[__gmp_i];                      \
            __gmp_r = __gmp_x OP 1;                        \
            (dst)[__gmp_i] = __gmp_r;                      \
            ++__gmp_i;                                     \
            if (!CB(__gmp_r, __gmp_x, 1))                  \
            {                                              \
               if ((src) != (dst))                         \
                  __GMPN_COPY_REST(dst, src, n, __gmp_i);  \
               (cout) = 0;                                 \
               break;                                      \
            }                                              \
         }                                                 \
      }                                                    \
      else                                                 \
      {                                                    \
         if ((src) != (dst))                               \
            __GMPN_COPY_REST(dst, src, n, 1);              \
         (cout) = 0;                                       \
      }                                                    \
   } while (0)
#endif

#if GMP_NAIL_BITS >= 1
#define __GMPN_AORS_1(cout, dst, src, n, v, OP, CB)        \
   do                                                      \
   {                                                       \
      mp_size_t __gmp_i;                                   \
      mp_limb_t __gmp_x, __gmp_r;                          \
                                                           \
      /* ASSERT ((n) >= 1); */                             \
      /* ASSERT (MPN_SAME_OR_SEPARATE_P (dst, src, n)); */ \
                                                           \
      __gmp_x = (src)[0];                                  \
      __gmp_r = __gmp_x OP(v);                             \
      (dst)[0] = __gmp_r & GMP_NUMB_MASK;                  \
      if (__gmp_r >> GMP_NUMB_BITS != 0)                   \
      {                                                    \
         (cout) = 1;                                       \
         for (__gmp_i = 1; __gmp_i < (n);)                 \
         {                                                 \
            __gmp_x = (src)[__gmp_i];                      \
            __gmp_r = __gmp_x OP 1;                        \
            (dst)[__gmp_i] = __gmp_r & GMP_NUMB_MASK;      \
            ++__gmp_i;                                     \
            if (__gmp_r >> GMP_NUMB_BITS == 0)             \
            {                                              \
               if ((src) != (dst))                         \
                  __GMPN_COPY_REST(dst, src, n, __gmp_i);  \
               (cout) = 0;                                 \
               break;                                      \
            }                                              \
         }                                                 \
      }                                                    \
      else                                                 \
      {                                                    \
         if ((src) != (dst))                               \
            __GMPN_COPY_REST(dst, src, n, 1);              \
         (cout) = 0;                                       \
      }                                                    \
   } while (0)
#endif

#define __GMPN_ADDCB(r, x, y) ((r) < (y))
#define __GMPN_SUBCB(r, x, y) ((x) < (y))

#define __GMPN_ADD_1(cout, dst, src, n, v) \
   __GMPN_AORS_1(cout, dst, src, n, v, +, __GMPN_ADDCB)
#define __GMPN_SUB_1(cout, dst, src, n, v) \
   __GMPN_AORS_1(cout, dst, src, n, v, -, __GMPN_SUBCB)

/* Compare {xp,size} and {yp,size}, setting "result" to positive, zero or
   negative.  size==0 is allowed.  On random data usually only one limb will
   need to be examined to get a result, so it's worth having it inline.  */
#define __GMPN_CMP(result, xp, yp, size)                              \
   do                                                                 \
   {                                                                  \
      mp_size_t __gmp_i;                                              \
      mp_limb_t __gmp_x, __gmp_y;                                     \
                                                                      \
      /* ASSERT ((size) >= 0); */                                     \
                                                                      \
      (result) = 0;                                                   \
      __gmp_i = (size);                                               \
      while (--__gmp_i >= 0)                                          \
      {                                                               \
         __gmp_x = (xp)[__gmp_i];                                     \
         __gmp_y = (yp)[__gmp_i];                                     \
         if (__gmp_x != __gmp_y)                                      \
         {                                                            \
            /* Cannot use __gmp_x - __gmp_y, may overflow an "int" */ \
            (result) = (__gmp_x > __gmp_y ? 1 : -1);                  \
            break;                                                    \
         }                                                            \
      }                                                               \
   } while (0)

#if defined(__GMPN_COPY) && !defined(__GMPN_COPY_REST)
#define __GMPN_COPY_REST(dst, src, size, start)                        \
   do                                                                  \
   {                                                                   \
      /* ASSERT ((start) >= 0); */                                     \
      /* ASSERT ((start) <= (size)); */                                \
      __GMPN_COPY((dst) + (start), (src) + (start), (size) - (start)); \
   } while (0)
#endif

/* Copy {src,size} to {dst,size}, starting at "start".  This is designed to
   keep the indexing dst[j] and src[j] nice and simple for __GMPN_ADD_1,
   __GMPN_ADD, etc.  */
#if !defined(__GMPN_COPY_REST)
#define __GMPN_COPY_REST(dst, src, size, start)               \
   do                                                         \
   {                                                          \
      mp_size_t __gmp_j;                                      \
      /* ASSERT ((size) >= 0); */                             \
      /* ASSERT ((start) >= 0); */                            \
      /* ASSERT ((start) <= (size)); */                       \
      /* ASSERT (MPN_SAME_OR_SEPARATE_P (dst, src, size)); */ \
      __GMP_CRAY_Pragma("_CRI ivdep");                        \
      for (__gmp_j = (start); __gmp_j < (size); __gmp_j++)    \
         (dst)[__gmp_j] = (src)[__gmp_j];                     \
   } while (0)
#endif

/* Enhancement: Use some of the smarter code from gmp-impl.h.  Maybe use
   mpn_copyi if there's a native version, and if we don't mind demanding
   binary compatibility for it (on targets which use it).  */

#if !defined(__GMPN_COPY)
#define __GMPN_COPY(dst, src, size) __GMPN_COPY_REST(dst, src, size, 0)
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpn_add)
#if !defined(__GMP_FORCE_mpn_add)
__GMP_EXTERN_INLINE
#endif
mp_limb_t
mpn_add(mp_ptr __gmp_wp, mp_srcptr __gmp_xp, mp_size_t __gmp_xsize, mp_srcptr __gmp_yp, mp_size_t __gmp_ysize)
{
   mp_limb_t __gmp_c;
   __GMPN_ADD(__gmp_c, __gmp_wp, __gmp_xp, __gmp_xsize, __gmp_yp, __gmp_ysize);
   return __gmp_c;
}
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpn_add_1)
#if !defined(__GMP_FORCE_mpn_add_1)
__GMP_EXTERN_INLINE
#endif
mp_limb_t
mpn_add_1(mp_ptr __gmp_dst, mp_srcptr __gmp_src, mp_size_t __gmp_size, mp_limb_t __gmp_n)
{
   mp_limb_t __gmp_c;
   __GMPN_ADD_1(__gmp_c, __gmp_dst, __gmp_src, __gmp_size, __gmp_n);
   return __gmp_c;
}
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpn_cmp)
#if !defined(__GMP_FORCE_mpn_cmp)
__GMP_EXTERN_INLINE
#endif
int mpn_cmp(mp_srcptr __gmp_xp, mp_srcptr __gmp_yp, mp_size_t __gmp_size)
{
   int __gmp_result;
   __GMPN_CMP(__gmp_result, __gmp_xp, __gmp_yp, __gmp_size);
   return __gmp_result;
}
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpn_zero_p)
#if !defined(__GMP_FORCE_mpn_zero_p)
__GMP_EXTERN_INLINE
#endif
int mpn_zero_p(mp_srcptr __gmp_p, mp_size_t __gmp_n)
{
   /* if (__GMP_LIKELY (__gmp_n > 0)) */
   do
   {
      if (__gmp_p[--__gmp_n] != 0)
         return 0;
   } while (__gmp_n != 0);
   return 1;
}
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpn_sub)
#if !defined(__GMP_FORCE_mpn_sub)
__GMP_EXTERN_INLINE
#endif
mp_limb_t
mpn_sub(mp_ptr __gmp_wp, mp_srcptr __gmp_xp, mp_size_t __gmp_xsize, mp_srcptr __gmp_yp, mp_size_t __gmp_ysize)
{
   mp_limb_t __gmp_c;
   __GMPN_SUB(__gmp_c, __gmp_wp, __gmp_xp, __gmp_xsize, __gmp_yp, __gmp_ysize);
   return __gmp_c;
}
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpn_sub_1)
#if !defined(__GMP_FORCE_mpn_sub_1)
__GMP_EXTERN_INLINE
#endif
mp_limb_t
mpn_sub_1(mp_ptr __gmp_dst, mp_srcptr __gmp_src, mp_size_t __gmp_size, mp_limb_t __gmp_n)
{
   mp_limb_t __gmp_c;
   __GMPN_SUB_1(__gmp_c, __gmp_dst, __gmp_src, __gmp_size, __gmp_n);
   return __gmp_c;
}
#endif

#if defined(__GMP_EXTERN_INLINE) || defined(__GMP_FORCE_mpn_neg)
#if !defined(__GMP_FORCE_mpn_neg)
__GMP_EXTERN_INLINE
#endif
mp_limb_t mpn_neg(mp_ptr __gmp_rp, mp_srcptr __gmp_up, mp_size_t __gmp_n)
{
   while (*__gmp_up == 0) /* Low zero limbs are unchanged by negation. */
   {
      *__gmp_rp = 0;
      if (!--__gmp_n) /* All zero */
         return 0;
      ++__gmp_up;
      ++__gmp_rp;
   }

   *__gmp_rp = (-*__gmp_up) & GMP_NUMB_MASK;

   if (--__gmp_n) /* Higher limbs get complemented. */
      mpn_com(++__gmp_rp, ++__gmp_up, __gmp_n);

   return 1;
}
#endif

/* Allow faster testing for negative, zero, and positive.  */
#define mpf_sgn(F) ((F)->_mp_size < 0 ? -1 : (F)->_mp_size > 0)
#define mpq_sgn(Q) ((Q)->_mp_num._mp_size < 0 ? -1 : (Q)->_mp_num._mp_size > 0)

/* When using GCC, optimize certain common comparisons.  */
#if defined(__GNUC__) && __GNUC__ >= 2
static inline int my_mpz_cmp_ui(mpz_struct *z, unsigned long int ui)
{
   if (__builtin_constant_p(ui) && ui == 0)
      return mpz_sgn(z);
   else
      return _mpz_cmp_ui(z, ui);
}
#define mpz_cmp_ui(Z, UI) my_mpz_cmp_ui(Z, UI)

static inline int my_mpz_cmp_si(mpz_struct *z, signed long int si)
{
   if (__builtin_constant_p(si) && si >= 0)
      return mpz_cmp_ui(z, (unsigned long)si);
   else
      return _mpz_cmp_si(z, si);
}
#define mpz_cmp_si(Z, SI) my_mpz_cmp_si(Z, SI)

static inline int my_mpq_cmp_ui(__mpq_struct *q, unsigned long nui, unsigned long dui)
{
   if (__builtin_constant_p(nui) && nui == 0)
      return mpq_sgn(q);
   else if (__builtin_constant_p(nui == dui) && nui == dui)
      return mpz_cmp(mpq_numref(q), mpq_denref(q));
   else
      return _mpq_cmp_ui(q, nui, dui);
}
#define mpq_cmp_ui(Q, NUI, DUI) my_mpq_cmp_ui(Q, NUI, DUI)

static inline int my_mpq_cmp_si(__mpq_struct *q, signed long int n, unsigned long d)
{
   if (__builtin_constant_p(n) && n >= 0)
      return mpq_cmp_ui(q, (unsigned long)n, d);
   else
      return _mpq_cmp_si(q, n, d);
}
#define mpq_cmp_si(q, n, d) my_mpq_cmp_si(q, n, d)

#else
#define mpz_cmp_ui(Z, UI) _mpz_cmp_ui(Z, UI)
#define mpz_cmp_si(Z, UI) _mpz_cmp_si(Z, UI)
#define mpq_cmp_ui(Q, NUI, DUI) _mpq_cmp_ui(Q, NUI, DUI)
#define mpq_cmp_si(q, n, d) _mpq_cmp_si(q, n, d)
#endif

/* Using "&" rather than "&&" means these can come out branch-free.  Every
   mpz_struct* has at least one limb allocated, so fetching the low limb is always
   allowed.  */
#define mpz_odd_p(z) (((z)->_mp_size != 0) & __GMP_CAST(int, (z)->_mp_d[0]))
#define mpz_even_p(z) (!mpz_odd_p(z))

/* Source-level compatibility with GMP 2 and earlier. */
#define mpn_divmod(qp, np, nsize, dp, dsize) \
   mpn_divrem(qp, __GMP_CAST(mp_size_t, 0), np, nsize, dp, dsize)

/* Source-level compatibility with GMP 1.  */
#define mpz_mdiv mpz_fdiv_q
#define mpz_mdivmod mpz_fdiv_qr
#define mpz_mmod mpz_fdiv_r
#define mpz_mdiv_ui mpz_fdiv_q_ui
#define mpz_mdivmod_ui(q, r, n, d) \
   (((r) == 0) ? mpz_fdiv_q_ui(q, n, d) : mpz_fdiv_qr_ui(q, r, n, d))
#define mpz_mmod_ui(r, n, d) \
   (((r) == 0) ? mpz_fdiv_ui(n, d) : mpz_fdiv_r_ui(r, n, d))

/* Useful synonyms, but not quite compatible with GMP 1.  */
#define mpz_div mpz_fdiv_q
#define mpz_divmod mpz_fdiv_qr
#define mpz_div_ui mpz_fdiv_q_ui
#define mpz_divmod_ui mpz_fdiv_qr_ui
#define mpz_div_2exp mpz_fdiv_q_2exp
#define mpz_mod_2exp mpz_fdiv_r_2exp

enum
{
   GMP_ERROR_NONE = 0,
   GMP_ERROR_UNSUPPORTED_ARGUMENT = 1,
   GMP_ERROR_DIVISION_BY_ZERO = 2,
   GMP_ERROR_SQRT_OF_NEGATIVE = 4,
   GMP_ERROR_INVALID_ARGUMENT = 8
};

/* Define CC and CFLAGS which were used to build this version of GMP */
#define __GMP_CC "gcc"
#define __GMP_CFLAGS "-O2 -pedantic -fomit-frame-pointer -m64 -mtune=nehalem -march=nehalem"

/* Major version number is the value of __GNU_MP__ too, above. */
#define __GNU_MP_VERSION 6
#define __GNU_MP_VERSION_MINOR 2
#define __GNU_MP_VERSION_PATCHLEVEL 1
#define __GNU_MP_RELEASE (__GNU_MP_VERSION * 10000 + __GNU_MP_VERSION_MINOR * 100 + __GNU_MP_VERSION_PATCHLEVEL)

#define __GMP_H__
#endif /* __GMP_H__ */
