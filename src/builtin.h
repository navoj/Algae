/*
   builtin.h -- Built-in functions.

   Copyright (C) 1994-2003  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: builtin.h,v 1.17 2003/11/24 23:59:02 ksh Exp $ */

/* This file holds info the parser needs to load
   builtin functions into its symbol table 
 */

#ifndef  BUILTIN_H
#define  BUILTIN_H	1

#include "function.h"

/*
 * Generic function pointer type for builtin dispatch.
 * The actual builtins have varying signatures; execute_bi() in function.c
 * casts this to the appropriate type before calling.
 */
typedef ENTITY *(*BI_FUNC_PTR)(void);

struct builtin
  {
    int min_args;
    int max_args;
    unsigned int arg_flags;
    /* A 1 bit means NULL allowed for that arg. */
    BI_FUNC_PTR bi;
  };

/* Forward declarations with proper prototypes */
extern ENTITY *bi_abs (ENTITY *p);
extern ENTITY *bi_acos (ENTITY *p);
extern ENTITY *bi_acosh (ENTITY *p);
extern ENTITY *bi_asin (ENTITY *p);
extern ENTITY *bi_asinh (ENTITY *p);
extern ENTITY *bi_atan (ENTITY *p);
extern ENTITY *bi_atanh (ENTITY *p);
extern ENTITY *bi_atan2 (ENTITY *y, ENTITY *x);
extern ENTITY *bi_atof (ENTITY *p);
extern ENTITY *bi_backsub (ENTITY *a, ENTITY *b);
extern ENTITY *bi_band (ENTITY *p);
extern ENTITY *bi_bdiag (ENTITY *a, ENTITY *nbr, ENTITY *nbc);
extern ENTITY *bi_breakpoint (int n, ENTITY *file, ENTITY *line_arg);
extern ENTITY *bi_btrans (ENTITY *a, ENTITY *nbr, ENTITY *nbc);
extern ENTITY *bi_builtin (ENTITY *obj, ENTITY *sym);
extern ENTITY *bi_cd (int n, ENTITY *path);
extern ENTITY *bi_ceil (ENTITY *p);
extern ENTITY *bi_char (ENTITY *e);
extern ENTITY *bi_chol (ENTITY *a);
extern ENTITY *bi_class (int n, ENTITY *p);
extern ENTITY *bi_close (ENTITY *fname);
extern ENTITY *bi_complement (ENTITY *l, ENTITY *r);
extern ENTITY *bi_conjugate (ENTITY *p);
extern ENTITY *bi_cram (ENTITY *shape, ENTITY *v);
extern ENTITY *bi_cos (ENTITY *p);
extern ENTITY *bi_cosh (ENTITY *p);
extern ENTITY *bi_dbclear (int n, ENTITY *file, ENTITY *line_arg);
extern ENTITY *bi_dbcont (void);
extern ENTITY *bi_dbstack (void);
extern ENTITY *bi_dbstatus (void);
extern ENTITY *bi_dbstep (int n, ENTITY *mode);
extern ENTITY *dense_entity (ENTITY *ip);
extern ENTITY *bi_diag (ENTITY *p);
extern ENTITY *bi_dice (ENTITY *s);
extern ENTITY *bi_digits (int n, ENTITY *prec);
extern ENTITY *bi_docstring (ENTITY *fn, ENTITY *text);
extern ENTITY *bi_eig (int n, ENTITY *a, ENTITY *b, ENTITY *c);
extern ENTITY *bi_equilibrate (ENTITY *p);
extern ENTITY *bi_erf (ENTITY *p);
extern ENTITY *bi_erfc (ENTITY *p);
extern ENTITY *bi_exception (void);
extern ENTITY *bi_exec (ENTITY *cmd);
extern ENTITY *bi_exit (int n, ENTITY *s);
extern ENTITY *bi_exp (ENTITY *p);
extern ENTITY *bi_exsparse (ENTITY *x);
extern ENTITY *bi_factor (ENTITY *a);
extern ENTITY *bi_fft (int n, ENTITY *p, ENTITY *d);
#if HAVE_FFTW
extern ENTITY *bi_fftw (int n, ENTITY *p, ENTITY *d);
#endif
extern ENTITY *bi_file (void);
extern ENTITY *bi_fill (ENTITY *shape, ENTITY *v);
extern ENTITY *bi_filter (int n, ENTITY *b, ENTITY *a, ENTITY *x, ENTITY *z);
extern ENTITY *bi_find (ENTITY *ap, ENTITY *bp);
extern ENTITY *bi_floor (ENTITY *p);
extern ENTITY *bi_form (ENTITY *shape, ENTITY *v);
extern ENTITY *bi_fprintf (int n, ENTITY **args);
extern ENTITY *bi_fread (int n, ENTITY *fname, ENTITY *length, ENTITY *type);
extern ENTITY *bi_fseek (int n, ENTITY *fname, ENTITY *offset, ENTITY *whence);
extern ENTITY *bi_ftell (int n, ENTITY *fname);
extern ENTITY *bi_full (ENTITY *p);
extern ENTITY *bi_fwrite (int n, ENTITY *fname, ENTITY *v, ENTITY *type);
extern ENTITY *bi_get (int n, ENTITY *fname);
extern ENTITY *bi_getdyn (int n, ENTITY *fp);
extern ENTITY *bi_getenv (ENTITY *name);
extern ENTITY *bi_getmat (int n, ENTITY *fname);
extern ENTITY *bi_gpskca (int n, ENTITY *p, ENTITY *b);
extern ENTITY *bi_help (int n, ENTITY *name);
extern ENTITY *bi_hermitian (ENTITY *p);
extern ENTITY *bi_ifft (int n, ENTITY *p, ENTITY *d);
#if HAVE_FFTW
extern ENTITY *bi_ifftw (int n, ENTITY *p, ENTITY *d);
#endif
extern ENTITY *bi_imag (ENTITY *p);
extern ENTITY *bi_integer (ENTITY *p);
extern ENTITY *bi_intersection (ENTITY *l, ENTITY *r);
extern ENTITY *bi_iram (ENTITY *dim, ENTITY *op_func, ENTITY *b_func, ENTITY *params, ENTITY *options);
extern ENTITY *bi_isort (ENTITY *p);
extern ENTITY *bi_leastsq (ENTITY *a, ENTITY *b);
extern ENTITY *bi_line (void);
extern ENTITY *bi_log (ENTITY *p);
extern ENTITY *bi_lose (ENTITY *ap, ENTITY *bp);
extern ENTITY *bi_matrix (int n, ENTITY *p);
extern ENTITY *bi_max (ENTITY *v);
extern ENTITY *member_names (ENTITY *t);
extern ENTITY *bi_min (ENTITY *v);
extern ENTITY *bi_mksparse (int nn, ENTITY *t);
extern ENTITY *bi_norm (int n, ENTITY *a, ENTITY *p);

#if HAVE_NPSOL
extern ENTITY *bi_npsol (int n, ENTITY *objective, ENTITY *start, ENTITY *constraints, ENTITY *options, ENTITY *state);
#endif

extern ENTITY *bi_print (int n, ENTITY *p, ENTITY *fname);
extern ENTITY *bi_printf (int n, ENTITY **args);
extern ENTITY *bi_put (int n, ENTITY *p, ENTITY *fname);
extern ENTITY *bi_putdyn (int n, ENTITY *t, ENTITY *fname);
extern ENTITY *bi_putmat (int n, ENTITY *t, ENTITY *fname);
extern ENTITY *bi_rand (int n, ENTITY *shape);
extern ENTITY *bi_read (int n, ENTITY *f);
extern ENTITY *bi_readnum (int n, ENTITY *f, ENTITY *v);
extern ENTITY *bi_real (ENTITY *p);
extern ENTITY *bi_round (ENTITY *p);
extern ENTITY *bi_scalar (int n, ENTITY *ip);
extern ENTITY *bi_set (int n, ENTITY *p);
extern ENTITY *bi_sin (ENTITY *p);
extern ENTITY *bi_sinh (ENTITY *p);
extern ENTITY *bi_sort (ENTITY *p);
extern ENTITY *bi_source (ENTITY *fname);
extern ENTITY *sparse_entity (ENTITY *ip);
extern ENTITY *bi_split (int n, ENTITY *str, ENTITY *sep);
extern ENTITY *bi_sprintf (int n, ENTITY **args);
extern ENTITY *bi_sqrt (ENTITY *p);
extern ENTITY *bi_srand (int n, ENTITY *s);
extern ENTITY *bi_strip (ENTITY *p);
extern ENTITY *bi_substr (int n, ENTITY *s, ENTITY *i, ENTITY *j);
extern ENTITY *bi_svd (int n, ENTITY *p, ENTITY *t);
extern ENTITY *bi_symmetric (ENTITY *p);
extern ENTITY *bi_system (ENTITY *);
extern ENTITY *bi_tan (ENTITY *p);
extern ENTITY *bi_tanh (ENTITY *p);
extern ENTITY *bi_test (ENTITY *p);
extern ENTITY *bi_time (void);
extern ENTITY *bi_tolower (ENTITY *e);
extern ENTITY *bi_toupper (ENTITY *e);
extern ENTITY *bi_transform (ENTITY *a, ENTITY *p);
extern ENTITY *tril_entity (int i, ENTITY *p, ENTITY *s);
extern ENTITY *triu_entity (int i, ENTITY *p, ENTITY *s);
extern ENTITY *bi_union (ENTITY *l, ENTITY *r);
extern ENTITY *bi_vector (int n, ENTITY *p);
extern ENTITY *bi_zero (ENTITY *a);

/* this macro must be turned on only once (in function.c) */
#ifdef BI_FUNCTION_INIT

#define BI_CAST(f)  ((BI_FUNC_PTR)(f))

static struct bi_init
  {
    char *name;
    BUILTIN bi;
  }
builtin_init_data[] =
{
  {"abs", {1, 1, 0, BI_CAST(bi_abs)}},
  {"acos", {1, 1, 0, BI_CAST(bi_acos)}},
  {"acosh", {1, 1, 0, BI_CAST(bi_acosh)}},
  {"asin", {1, 1, 0, BI_CAST(bi_asin)}},
  {"asinh", {1, 1, 0, BI_CAST(bi_asinh)}},
  {"atan", {1, 1, 0, BI_CAST(bi_atan)}},
  {"atanh", {1, 1, 0, BI_CAST(bi_atanh)}},
  {"atan2", {2, 2, 0, BI_CAST(bi_atan2)}},
  {"atof", {1, 1, 0, BI_CAST(bi_atof)}},
  {"backsub", {2, 2, 0, BI_CAST(bi_backsub)}},
  {"band", {1, 1, 0, BI_CAST(bi_band)}},
  {"bdiag", {3, 3, 0, BI_CAST(bi_bdiag)}},
  {"breakpoint", {1, 2, 0, BI_CAST(bi_breakpoint)}},
  {"btrans", {3, 3, 0, BI_CAST(bi_btrans)}},
  {"builtin", {2, 2, 0, BI_CAST(bi_builtin)}},
  {"cd", {0, 1, 01, BI_CAST(bi_cd)}},
  {"ceil", {1, 1, 0, BI_CAST(bi_ceil)}},
  {"char", {1, 1, 0, BI_CAST(bi_char)}},
  {"chol", {1, 1, 0, BI_CAST(bi_chol)}},
  {"class", {0, 1, 01, BI_CAST(bi_class)}},
  {"close", {1, 1, 0, BI_CAST(bi_close)}},
  {"complement", {2, 2, 0, BI_CAST(bi_complement)}},
  {"conj", {1, 1, 0, BI_CAST(bi_conjugate)}},
  {"cram", {2, 2, 01, BI_CAST(bi_cram)}},
  {"cos", {1, 1, 0, BI_CAST(bi_cos)}},
  {"cosh", {1, 1, 0, BI_CAST(bi_cosh)}},
  {"dbclear", {0, 2, 03, BI_CAST(bi_dbclear)}},
  {"dbcont", {0, 0, 0, BI_CAST(bi_dbcont)}},
  {"dbstack", {0, 0, 0, BI_CAST(bi_dbstack)}},
  {"dbstatus", {0, 0, 0, BI_CAST(bi_dbstatus)}},
  {"dbstep", {0, 1, 01, BI_CAST(bi_dbstep)}},
  {"dense", {1, 1, 0, BI_CAST(dense_entity)}},
  {"diag", {1, 1, 0, BI_CAST(bi_diag)}},
  {"dice", {1, 1, 0, BI_CAST(bi_dice)}},
  {"digits", {0, 1, 01, BI_CAST(bi_digits)}},
  {"docstring", {2, 2, 0, BI_CAST(bi_docstring)}},
  {"eig", {1, 3, 06, BI_CAST(bi_eig)}},
  {"equilibrate", {1, 1, 0, BI_CAST(bi_equilibrate)}},
  {"erf", {1, 1, 0, BI_CAST(bi_erf)}},
  {"erfc", {1, 1, 0, BI_CAST(bi_erfc)}},
  {"exception", {0, 0, 0, BI_CAST(bi_exception)}},
  {"exec", {1, 1, 0, BI_CAST(bi_exec)}},
  {"exit", {0, 1, 01, BI_CAST(bi_exit)}},
  {"exp", {1, 1, 0, BI_CAST(bi_exp)}},
  {"exsparse", {1, 1, 0, BI_CAST(bi_exsparse)}},
  {"factor", {1, 1, 0, BI_CAST(bi_factor)}},
#if HAVE_FFTW
#if USE_BOTH_FFT
  {"fft", {1, 2, 02, BI_CAST(bi_fft)}},
  {"fftw", {1, 2, 02, BI_CAST(bi_fftw)}},
#else
  {"fft", {1, 2, 02, BI_CAST(bi_fftw)}},
#endif
#else
  {"fft", {1, 2, 02, BI_CAST(bi_fft)}},
#endif
  {"file", {0, 0, 0, BI_CAST(bi_file)}},
  {"fill", {2, 2, 01, BI_CAST(bi_fill)}},
  {"filter", {3, 4, 010, BI_CAST(bi_filter)}},
  {"find", {2, 2, 0, BI_CAST(bi_find)}},
  {"floor", {1, 1, 0, BI_CAST(bi_floor)}},
  {"form", {2, 2, 01, BI_CAST(bi_form)}},
  {"fprintf", {2, 99, ~(unsigned) 0, BI_CAST(bi_fprintf)}},
  {"fread", {0, 3, 07, BI_CAST(bi_fread)}},
  {"fseek", {2, 3, 04, BI_CAST(bi_fseek)}},
  {"ftell", {0, 1, 01, BI_CAST(bi_ftell)}},
  {"full", {1, 1, 0, BI_CAST(bi_full)}},
  {"fwrite", {0, 3, 05, BI_CAST(bi_fwrite)}},
  {"get", {0, 1, 0, BI_CAST(bi_get)}},
  {"getdyn", {0, 1, 01, BI_CAST(bi_getdyn)}},
  {"getenv", {1, 1, 0, BI_CAST(bi_getenv)}},
  {"getmat", {0, 1, 01, BI_CAST(bi_getmat)}},
  {"gpskca", {1, 2, 02, BI_CAST(bi_gpskca)}},
  {"help", {0, 1, 01, BI_CAST(bi_help)}},
  {"hermitian", {1, 1, 0, BI_CAST(bi_hermitian)}},
#if HAVE_FFTW
#if USE_BOTH_FFT
  {"ifft", {1, 2, 02, BI_CAST(bi_ifft)}},
  {"ifftw", {1, 2, 02, BI_CAST(bi_ifftw)}},
#else
  {"ifft", {1, 2, 02, BI_CAST(bi_ifftw)}},
#endif
#else
  {"ifft", {1, 2, 02, BI_CAST(bi_ifft)}},
#endif
  {"imag", {1, 1, 0, BI_CAST(bi_imag)}},
  {"integer", {1, 1, 0, BI_CAST(bi_integer)}},
  {"intersection", {2, 2, 0, BI_CAST(bi_intersection)}},
  {"iram", {5, 5, 034, BI_CAST(bi_iram)}},
  {"isort", {1, 1, 0, BI_CAST(bi_isort)}},
  {"leastsq", {2, 2, 0, BI_CAST(bi_leastsq)}},
  {"line", {0, 0, 0, BI_CAST(bi_line)}},
  {"log", {1, 1, 0, BI_CAST(bi_log)}},
  {"lose", {2, 2, 0, BI_CAST(bi_lose)}},
  {"matrix", {0, 1, 01, BI_CAST(bi_matrix)}},
  {"imax", {1, 1, 0, BI_CAST(bi_max)}},
  {"members", {1, 1, 0, BI_CAST(member_names)}},
  {"imin", {1, 1, 0, BI_CAST(bi_min)}},
  {"mksparse", {0, 1, 01, BI_CAST(bi_mksparse)}},
  {"norm", {1, 2, 0, BI_CAST(bi_norm)}},
#if HAVE_NPSOL
  {"npsol", {4, 5, 0, BI_CAST(bi_npsol)}},
#endif
  {"print", {1, 2, 0, BI_CAST(bi_print)}},
  {"printf", {1, 99, ~(unsigned) 0, BI_CAST(bi_printf)}},
  {"put", {1, 2, 0, BI_CAST(bi_put)}},
  {"putdyn", {1, 2, 02, BI_CAST(bi_putdyn)}},
  {"putmat", {1, 2, 02, BI_CAST(bi_putmat)}},
  {"rand", {0, 1, 01, BI_CAST(bi_rand)}},
  {"read", {0, 1, 01, BI_CAST(bi_read)}},
  {"readnum", {0, 2, 03, BI_CAST(bi_readnum)}},
  {"real", {1, 1, 0, BI_CAST(bi_real)}},
  {"round", {1, 1, 0, BI_CAST(bi_round)}},
  {"scalar", {0, 1, 01, BI_CAST(bi_scalar)}},
  {"set", {0, 1, 0, BI_CAST(bi_set)}},
  {"sin", {1, 1, 0, BI_CAST(bi_sin)}},
  {"sinh", {1, 1, 0, BI_CAST(bi_sinh)}},
  {"sort", {1, 1, 0, BI_CAST(bi_sort)}},
  {"source", {1, 1, 0, BI_CAST(bi_source)}},
  {"sparse", {1, 1, 0, BI_CAST(sparse_entity)}},
  {"split", {1, 2, 02, BI_CAST(bi_split)}},
  {"sprintf", {1, 99, ~(unsigned) 0, BI_CAST(bi_sprintf)}},
  {"sqrt", {1, 1, 0, BI_CAST(bi_sqrt)}},
  {"srand", {0, 1, 0, BI_CAST(bi_srand)}},
  {"strip", {1, 1, 0, BI_CAST(bi_strip)}},
  {"substr", {2, 3, 04, BI_CAST(bi_substr)}},
  {"svd", {1, 2, 02, BI_CAST(bi_svd)}},
  {"symmetric", {1, 1, 0, BI_CAST(bi_symmetric)}},
  {"system", {1, 1, 0, BI_CAST(bi_system)}},
  {"tan", {1, 1, 0, BI_CAST(bi_tan)}},
  {"tanh", {1, 1, 0, BI_CAST(bi_tanh)}},
  {"test", {1, 1, 01, BI_CAST(bi_test)}},
  {"time", {0, 0, 0, BI_CAST(bi_time)}},
  {"tolower", {1, 1, 0, BI_CAST(bi_tolower)}},
  {"toupper", {1, 1, 0, BI_CAST(bi_toupper)}},
  {"transform", {2, 2, 0, BI_CAST(bi_transform)}},
  {"tril", {1, 2, 02, BI_CAST(tril_entity)}},
  {"triu", {1, 2, 02, BI_CAST(triu_entity)}},
  {"union", {2, 2, 0, BI_CAST(bi_union)}},
  {"vector", {0, 1, 01, BI_CAST(bi_vector)}},
  {"zero", {0, 1, 01, BI_CAST(bi_zero)}},
  {NULL, {0, 0, 0, NULL}}
};

#endif /* BI_FUNCTION_INIT */


ENTITY *PROTO (execute_bi, (BUILTIN *, int, DATUM *));

#endif
