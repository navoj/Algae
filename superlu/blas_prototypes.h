#ifndef BLAS_PROTOTYPES_H
#define BLAS_PROTOTYPES_H

/* Double real BLAS */
extern void dtrsv_(const char *uplo, const char *trans, const char *diag,
                   const int *n, const double *a, const int *lda,
                   double *x, const int *incx);
extern void dgemv_(const char *trans, const int *m, const int *n,
                   const double *alpha, const double *a, const int *lda,
                   const double *x, const int *incx, const double *beta,
                   double *y, const int *incy);
extern void dtrsm_(const char *side, const char *uplo, const char *transa,
                   const char *diag, const int *m, const int *n,
                   const double *alpha, const double *a, const int *lda,
                   double *b, const int *ldb);
extern void dgemm_(const char *transa, const char *transb,
                   const int *m, const int *n, const int *k,
                   const double *alpha, const double *a, const int *lda,
                   const double *b, const int *ldb, const double *beta,
                   double *c, const int *ldc);

/* Double complex BLAS */
extern void ztrsv_(const char *uplo, const char *trans, const char *diag,
                   const int *n, const void *a, const int *lda,
                   void *x, const int *incx);
extern void zgemv_(const char *trans, const int *m, const int *n,
                   const void *alpha, const void *a, const int *lda,
                   const void *x, const int *incx, const void *beta,
                   void *y, const int *incy);
extern void ztrsm_(const char *side, const char *uplo, const char *transa,
                   const char *diag, const int *m, const int *n,
                   const void *alpha, const void *a, const int *lda,
                   void *b, const int *ldb);
extern void zgemm_(const char *transa, const char *transb,
                   const int *m, const int *n, const int *k,
                   const void *alpha, const void *a, const int *lda,
                   const void *b, const int *ldb, const void *beta,
                   void *c, const int *ldc);
extern void zcopy_(const int *n, const void *x, const int *incx,
                   void *y, const int *incy);

/* LAPACK error handler */
extern int xerbla_(char *srname, int *info);

#endif
