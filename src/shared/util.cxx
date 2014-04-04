/*Copyright (c) 2011, Edgar Solomonik, all rights reserved.*/

#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include "assert.h"
#include "util.h"
#include "../../include/ctf.hpp"

long_int CTF_total_flop_count = 0;

void CTF_flops_add(long_int n){
  CTF_total_flop_count+=n;
}

long_int CTF_get_flops(){
  return CTF_total_flop_count;
}


#if (defined BGP || defined BGQ)
#define UTIL_ZGEMM      zgemm
#define UTIL_DGEMM      dgemm
#define UTIL_DAXPY      daxpy
#define UTIL_ZAXPY      zaxpy
#define UTIL_DCOPY      dcopy
#define UTIL_ZCOPY      zcopy
#define UTIL_DSCAL      dscal
#define UTIL_ZSCAL      zscal
#define UTIL_DDOT       ddot
#else
#define UTIL_ZGEMM      zgemm_
#define UTIL_DGEMM      dgemm_
#define UTIL_DAXPY      daxpy_
#define UTIL_ZAXPY      zaxpy_
#define UTIL_DCOPY      dcopy_
#define UTIL_ZCOPY      zcopy_
#define UTIL_DSCAL      dscal_
#define UTIL_ZSCAL      zscal_
#define UTIL_DDOT       ddot_
#endif

#ifdef USE_JAG
extern "C"
void jag_zgemm( char *, char *,
                int *,  int *,
                int *,  double *,
                double *,       int *,
                double *,       int *,
                double *,       double *,
                                int *);

#endif

extern "C"
void UTIL_DGEMM(const char *,   const char *,
                const int *,    const int *,
                const int *,    const double *,
                const double *, const int *,
                const double *, const int *,
                const double *, double *,
                                const int *);

extern "C"
void UTIL_ZGEMM(const char *,   const char *,
                const int *,    const int *,
                const int *,    const std::complex<double> *,
                const std::complex<double> *,   const int *,
                const std::complex<double> *,   const int *,
                const std::complex<double> *,   std::complex<double> *,
                                const int *);


extern "C"
void UTIL_DAXPY(const int * n,          double * dA,
                const double * dX,      const int * incX,
                double * dY,            const int * incY);

extern "C"
void UTIL_ZAXPY(const int * n,                          std::complex<double> * dA,
                const std::complex<double> * dX,        const int * incX,
                std::complex<double> * dY,              const int * incY);

extern "C"
void UTIL_DCOPY(const int * n,
                const double * dX,      const int * incX,
                double * dY,            const int * incY);

extern "C"
void UTIL_ZCOPY(const int * n,
                const std::complex<double> * dX,        const int * incX,
                std::complex<double> * dY,              const int * incY);


extern "C"
void UTIL_DSCAL(const int *n,           double *dA,
                double * dX,      const int *incX);

extern "C"
void UTIL_ZSCAL(const int *n,           std::complex<double> *dA,
                std::complex<double> * dX,      const int *incX);

extern "C"
double UTIL_DDOT(const int * n,         const double * dX,      
                 const int * incX,      const double * dY,      
                 const int * incY);

void cdgemm(const char transa,  const char transb,
            const int m,        const int n,
            const int k,        const double a,
            const double * A,   const int lda,
            const double * B,   const int ldb,
            const double b,     double * C,
                                const int ldc){
  UTIL_DGEMM(&transa, &transb, &m, &n, &k, &a, A,
             &lda, B, &ldb, &b, C, &ldc);
}

void czgemm(const char transa,  const char transb,
            const int m,        const int n,
            const int k,        const std::complex<double> a,
            const std::complex<double> * A,     const int lda,
            const std::complex<double> * B,     const int ldb,
            const std::complex<double> b,       std::complex<double> * C,
                                const int ldc){
#ifdef USE_JAG
  jag_zgemm((char*)&transa, (char*)&transb, (int*)&m, (int*)&n, (int*)&k, (double*)&a, (double*)A,
             (int*)&lda, (double*)B, (int*)&ldb, (double*)&b, (double*)C, (int*)&ldc);
#else
  UTIL_ZGEMM(&transa, &transb, &m, &n, &k, &a, A,
             &lda, B, &ldb, &b, C, &ldc);
#endif
}

void czaxpy(const int n,        
            std::complex<double> dA,
            const std::complex<double> * dX,
            const int incX,
            std::complex<double> * dY,
            const int incY){
  UTIL_ZAXPY(&n, &dA, dX, &incX, dY, &incY);
}


void cdaxpy(const int n,        double dA,
            const double * dX,  const int incX,
            double * dY,        const int incY){
  UTIL_DAXPY(&n, &dA, dX, &incX, dY, &incY);
}

void czcopy(const int n,
            const std::complex<double> * dX,
            const int incX,
            std::complex<double> * dY,
            const int incY){
  UTIL_ZCOPY(&n, dX, &incX, dY, &incY);
}


void cdcopy(const int n,
            const double * dX,  const int incX,
            double * dY,        const int incY){
  UTIL_DCOPY(&n, dX, &incX, dY, &incY);
}

void cdscal(const int n,        double dA,
            double * dX,  const int incX){
  UTIL_DSCAL(&n, &dA, dX, &incX);
}

void czscal(const int n,        std::complex<double> dA,
            std::complex<double> * dX,  const int incX){
  UTIL_ZSCAL(&n, &dA, dX, &incX);
}


double cddot(const int n,       const double *dX,
             const int incX,    const double *dY,
             const int incY){
  return UTIL_DDOT(&n, dX, &incX, dY, &incY);
}


/**
 * \brief prints matrix in 2D
 * \param[in] M matrix
 * \param[in] n number of rows
 * \param[in] m number of columns
 */
void print_matrix(double *M, int n, int m){
  int i,j;
  for (i = 0; i < n; i++){
    for (j = 0; j < m; j++){
      printf("%lf ", M[i+j*n]);
    }
    printf("\n");
  }
}

/* abomination */
double util_dabs(double x){
  if (x >= 0.0) return x;
  return -x;
}

#ifdef COMM_TIME
/** 
 * \brief ugliest timer implementation on Earth
 * \param[in] end the type of operation this function should do (oh god why?)
 * \param[in] cdt the communicator
 * \param[in] p the number of processors
 * \param[in] iter the number of iterations if relevant
 * \param[in] myRank your rank if relevant
 */
void __CM(const int     end, 
          const CommData cdt, 
          const int     p, 
          const int     iter, 
          const int     myRank){
  static volatile double __commTime     =0.0;
  static volatile double __commTimeDelta=0.0;
  static volatile double __idleTime     =0.0;
  static volatile double __idleTimeDelta=0.0;
  if (end == 0){
    __idleTimeDelta = TIME_SEC();       
    COMM_BARRIER(cdt); 
    __idleTime += TIME_SEC() - __idleTimeDelta;
    __commTimeDelta = TIME_SEC(); 
  }
  else if (end == 1){
    __commTime += TIME_SEC() - __commTimeDelta;
  } else if (end == 2) {
    MPI_Reduce((void*)&__commTime, (void*)&__commTimeDelta, 1, COMM_DOUBLE_T, COMM_OP_SUM, 0, cdt.cm); 
    __commTime = __commTimeDelta/p;
    if (myRank == 0)
      printf("%lf seconds spent doing communication on average per iteration\n", __commTime/iter); 

    MPI_Reduce((void*)&__idleTime, (void*)&__idleTimeDelta, 1,
            COMM_DOUBLE_T, COMM_OP_SUM, 0, cdt.cm);
    __idleTime = __idleTimeDelta/p;
    if (myRank == 0)
      printf("%lf seconds spent idle per iteration\n", __idleTime/iter); 
  } else if (end == 3){
    __commTime =0.0;
    __idleTime =0.0;
  } else if (end == 4){
    MPI_Irecv(NULL,0,MPI_CHAR,iter,myRank,cdt.cm,&(cdt.req[myRank]));
  } else if (end == 5){
    __idleTimeDelta =TIME_SEC();
    MPI_Send(NULL,0,MPI_CHAR,iter,myRank,cdt.cm);
    __idleTime += TIME_SEC() - __idleTimeDelta;
    __commTimeDelta = TIME_SEC(); 
  } else if (end == 6){
    MPI_Status __stat;
    __idleTimeDelta =TIME_SEC();
    MPI_Wait(&(cdt.req[myRank]),&__stat);
    __idleTime += TIME_SEC() - __idleTimeDelta;
    __commTimeDelta = TIME_SEC(); 
  }
} 

#endif
/**
 * \brief computes the size of a tensor in NOT HOLLOW packed symmetric layout
 * \param[in] ndim tensor dimension
 * \param[in] len tensor edge _elngths
 * \param[in] sym tensor symmetries
 * \return size of tensor in packed layout
 */
long_int sy_packed_size(const int ndim, const int* len, const int* sym){
  int i, k, mp;
  long_int size, tmp;

  if (ndim == 0) return 1;

  k = 1;
  tmp = 1;
  size = 1;
  if (ndim > 0)
    mp = len[0];
  else
    mp = 1;
  for (i = 0;i < ndim;i++){
    tmp = (tmp * mp) / k;
    k++;
    mp ++;
    
    if (sym[i] == 0){
      size *= tmp;
      k = 1;
      tmp = 1;
      if (i < ndim - 1) mp = len[i + 1];
    }
  }
  size *= tmp;

  return size;
}




/**
 * \brief computes the size of a tensor in packed symmetric layout
 * \param[in] ndim tensor dimension
 * \param[in] len tensor edge _elngths
 * \param[in] sym tensor symmetries
 * \return size of tensor in packed layout
 */
long_int packed_size(const int ndim, const int* len, const int* sym){

  int i, k, mp;
  long_int size, tmp;

  if (ndim == 0) return 1;

  k = 1;
  tmp = 1;
  size = 1;
  if (ndim > 0)
    mp = len[0];
  else
    mp = 1;
  for (i = 0;i < ndim;i++){
    tmp = (tmp * mp) / k;
    k++;
    if (sym[i] != 1)
      mp--;
    else
      mp ++;
    
    if (sym[i] == 0){
      size *= tmp;
      k = 1;
      tmp = 1;
      if (i < ndim - 1) mp = len[i + 1];
    }
  }
  size *= tmp;

  return size;
}

/*
 * \brief calculates dimensional indices corresponding to a symmetric-packed index
 *        For each symmetric (SH or AS) group of size sg we have
 *          idx = n*(n-1)*...*(n-sg) / d*(d-1)*...
 *        therefore (idx*sg!)^(1/sg) >= n-sg
 *        or similarly in the SY case ... >= n
 *
 * \param[in] ndim number of dimensions in the tensor 
 * \param[in] lens edge lengths 
 * \param[in] sym symmetry
 * \param[in] idx index in the global tensor, in packed format
 * \param[out] idx_arr preallocated to size ndim, computed to correspond to idx
 */
void calc_idx_arr(int         ndim,
                  int const * lens,
                  int const * sym,
                  long_int    idx,
                  int *       idx_arr){
  long_int idx_rem = idx;
  memset(idx_arr, 0, ndim*sizeof(int));
  for (int dim=ndim-1; dim>=0; dim--){
    if (idx_rem == 0) break;
    if (dim == 0 || sym[dim-1] == NS){
      long_int lda = packed_size(dim, lens, sym);
      idx_arr[dim] = idx_rem/lda;
      idx_rem -= idx_arr[dim]*lda;
    } else {
      int plen[dim+1];
      memcpy(plen, lens, (dim+1)*sizeof(int));
      int sg = 2;
      int fsg = 2;
      while (dim >= sg && sym[dim-sg] != NS) { sg++; fsg*=sg; }
      long_int lda = packed_size(dim-sg+1, lens, sym);
      double fsg_idx = (((double)idx_rem)*fsg)/lda;
      int kidx = (int)pow(fsg_idx,1./sg);
      //if (sym[dim-1] != SY) 
      kidx += sg+1;
      int mkidx = kidx;
#if DEBUG >= 1
      for (int idim=dim-sg+1; idim<=dim; idim++){
        plen[idim] = mkidx+1;
      }
      long_int smidx = packed_size(dim+1, plen, sym);
      LIBT_ASSERT(smidx > idx_rem);
#endif
      long_int midx = 0;
      for (; mkidx >= 0; mkidx--){
        for (int idim=dim-sg+1; idim<=dim; idim++){
          plen[idim] = mkidx;
        }
        midx = packed_size(dim+1, plen, sym);
        if (midx <= idx_rem) break;
      }
      if (midx == 0) mkidx = 0;
      idx_arr[dim] = mkidx;
      idx_rem -= midx;
    }
  }
  LIBT_ASSERT(idx_rem == 0);
}

/**
 * \brief computes the size of a tensor in packed symmetric layout
 * \param[in] n a positive number
 * \param[out] nfactor number of factors in n
 * \param[out] factor array of length nfactor, corresponding to factorization of n
 */
void factorize(int n, int *nfactor, int **factor){
  int tmp, nf, i;
  int * ff;
  tmp = n;
  nf = 0;
  while (tmp > 1){
    for (i=2; i<=n; i++){
      if (tmp % i == 0){
        nf++;
        tmp = tmp/i;
        break;
      }
    }
  }
  if (nf == 0){
    *nfactor = nf;
  } else {
    ff  = (int*)CTF_alloc(sizeof(int)*nf);
    tmp = n;
    nf = 0;
    while (tmp > 1){
      for (i=2; i<=n; i++){
        if (tmp % i == 0){
          ff[nf] = i;
          nf++;
          tmp = tmp/i;
          break;
        }
      }
    }
    *factor = ff;
    *nfactor = nf;
  }
}

int conv_idx(int const  ndim,
             char const *  cidx,
             int **     iidx){
  int i, j, n;
  char c;

  *iidx = (int*)CTF_alloc(sizeof(int)*ndim);

  n = 0;
  for (i=0; i<ndim; i++){
    c = cidx[i];
    for (j=0; j<i; j++){
      if (c == cidx[j]){
        (*iidx)[i] = (*iidx)[j];
        break;
      }
    }
    if (j==i){
      (*iidx)[i] = n;
      n++;
    }
  }
  return n;
}

int  conv_idx(int const         ndim_A,
              char const *         cidx_A,
              int **            iidx_A,
              int const         ndim_B,
              char const *         cidx_B,
              int **            iidx_B){
  int i, j, n;
  char c;

  *iidx_B = (int*)CTF_alloc(sizeof(int)*ndim_B);

  n = conv_idx(ndim_A, cidx_A, iidx_A);
  for (i=0; i<ndim_B; i++){
    c = cidx_B[i];
    for (j=0; j<ndim_A; j++){
      if (c == cidx_A[j]){
        (*iidx_B)[i] = (*iidx_A)[j];
        break;
      }
    }
    if (j==ndim_A){
      for (j=0; j<i; j++){
        if (c == cidx_B[j]){
          (*iidx_B)[i] = (*iidx_B)[j];
          break;
        }
      }
      if (j==i){
        (*iidx_B)[i] = n;
        n++;
      }
    }
  }
  return n;
}


int  conv_idx(int const         ndim_A,
              char const *         cidx_A,
              int **            iidx_A,
              int const         ndim_B,
              char const *         cidx_B,
              int **            iidx_B,
              int const         ndim_C,
              char const *         cidx_C,
              int **            iidx_C){
  int i, j, n;
  char c;

  *iidx_C = (int*)CTF_alloc(sizeof(int)*ndim_C);

  n = conv_idx(ndim_A, cidx_A, iidx_A,
               ndim_B, cidx_B, iidx_B);

  for (i=0; i<ndim_C; i++){
    c = cidx_C[i];
    for (j=0; j<ndim_B; j++){
      if (c == cidx_B[j]){
        (*iidx_C)[i] = (*iidx_B)[j];
        break;
      }
    }
    if (j==ndim_B){
      for (j=0; j<ndim_A; j++){
        if (c == cidx_A[j]){
          (*iidx_C)[i] = (*iidx_A)[j];
          break;
        }
      }
      if (j==ndim_A){
        for (j=0; j<i; j++){
          if (c == cidx_C[j]){
            (*iidx_C)[i] = (*iidx_C)[j];
            break;
          }
        }
        if (j==i){
          (*iidx_C)[i] = n;
          n++;
        }
      }
    }
  }
  return n;
}

void conv_idx(int          ndim,
              int const *  lens,
              long_int     idx,
              int *        idx_arr){
  int i;
  long_int cidx = idx;
  for (i=0; i<ndim; i++){
    idx_arr[i] = cidx%lens[i];
    cidx = cidx/lens[i];
  }
}

void conv_idx(int          ndim,
              int const *  lens,
              long_int     idx,
              int **       idx_arr){
  (*idx_arr) = (int*)CTF_alloc(ndim*sizeof(int));
  conv_idx(ndim, lens, idx, *idx_arr);
}

void conv_idx(int          ndim,
              int const *  lens,
              int const *  idx_arr,
              long_int *   idx){
  int i;
  long_int lda = 1;
  *idx = 0;
  for (i=0; i<ndim; i++){
    (*idx) += idx_arr[i]*lda;
    lda *= lens[i];
  }

}

/**
 * \brief performs all-to-all-v with 64-bit integer counts and offset on arbitrary
 *        length types (datum_size), and uses point-to-point when all-to-all-v sparse
 * \param[in] send_buffer data to send
 * \param[in] send_counts number of datums to send to each process
 * \param[in] send_displs displacements of datum sets in sen_buffer
 * \param[in] datum_size size of MPI_datatype to use
 * \param[in,out] recv_buffer data to recv
 * \param[in] recv_counts number of datums to recv to each process
 * \param[in] recv_displs displacements of datum sets in sen_buffer
 * \param[in] cdt wrapper for communicator
 */
void CTF_all_to_allv(void *           send_buffer, 
                     long_int const * send_counts,
                     long_int const * send_displs,
                     long_int         datum_size,
                     void *           recv_buffer, 
                     long_int const * recv_counts,
                     long_int const * recv_displs,
                     CommData_t       cdt){
  int num_nnz_trgt = 0;
  int num_nnz_recv = 0;
  int np = cdt.np;
  for (int p=0; p<np; p++){
    if (send_counts[p] != 0) num_nnz_trgt++;
    if (recv_counts[p] != 0) num_nnz_recv++;
  }
  double frac_nnz = ((double)num_nnz_trgt)/np;
  double tot_frac_nnz;
  ALLREDUCE(&frac_nnz, &tot_frac_nnz, 1, MPI_DOUBLE, MPI_SUM, cdt);
  tot_frac_nnz = tot_frac_nnz / np;

  long_int max_displs = MAX(recv_displs[np-1], send_displs[np-1]);
  long_int tot_max_displs;
  
  ALLREDUCE(&max_displs, &tot_max_displs, 1, MPI_INT64_T, MPI_MAX, cdt);
  
  if (tot_max_displs >= INT32_MAX ||
      (datum_size != 4 && datum_size != 8 && datum_size != 16) ||
      (tot_frac_nnz <= .25 && tot_frac_nnz*np < 100)){
    MPI_Request reqs[num_nnz_recv+num_nnz_trgt];
    MPI_Status stat[num_nnz_recv+num_nnz_trgt];
    int nnr = 0;
    for (int p=0; p<np; p++){
      if (recv_counts[p] != 0){
        MPI_Irecv(((char*)recv_buffer)+recv_displs[p]*datum_size, 
                  datum_size*recv_counts[p], 
                  MPI_CHAR, p, p, cdt.cm, reqs+nnr);
        nnr++;
      } 
    }
    int nns = 0;
    for (int lp=0; lp<np; lp++){
      int p = (lp+cdt.rank)%np;
      if (send_counts[p] != 0){
        MPI_Isend(((char*)send_buffer)+send_displs[p]*datum_size, 
                  datum_size*send_counts[p], 
                  MPI_CHAR, p, cdt.rank, cdt.cm, reqs+nnr+nns);
        nns++;
      } 
    }
    MPI_Waitall(num_nnz_recv+num_nnz_trgt, reqs, stat);
  } else {
    int * i32_send_counts, * i32_send_displs;
    int * i32_recv_counts, * i32_recv_displs;

    
    CTF_mst_alloc_ptr(np*sizeof(int), (void**)&i32_send_counts);
    CTF_mst_alloc_ptr(np*sizeof(int), (void**)&i32_send_displs);
    CTF_mst_alloc_ptr(np*sizeof(int), (void**)&i32_recv_counts);
    CTF_mst_alloc_ptr(np*sizeof(int), (void**)&i32_recv_displs);

    for (int p=0; p<np; p++){
      i32_send_counts[p] = send_counts[p];
      i32_send_displs[p] = send_displs[p];
      i32_recv_counts[p] = recv_counts[p];
      i32_recv_displs[p] = recv_displs[p];
    }
    switch (datum_size){
      case 4:
        ALL_TO_ALLV(send_buffer, i32_send_counts, i32_send_displs, MPI_FLOAT,
                    recv_buffer, i32_recv_counts, i32_recv_displs, MPI_FLOAT, cdt);
        break;
      case 8:
        ALL_TO_ALLV(send_buffer, i32_send_counts, i32_send_displs, MPI_DOUBLE,
                    recv_buffer, i32_recv_counts, i32_recv_displs, MPI_DOUBLE, cdt);
        break;
      case 16:
        ALL_TO_ALLV(send_buffer, i32_send_counts, i32_send_displs, MPI_DOUBLE_COMPLEX,
                    recv_buffer, i32_recv_counts, i32_recv_displs, MPI_DOUBLE_COMPLEX, cdt);
        break;
      default: 
        ABORT;
        break;
    }
    CTF_free(i32_send_counts);
    CTF_free(i32_send_displs);
    CTF_free(i32_recv_counts);
    CTF_free(i32_recv_displs);
  }
}

