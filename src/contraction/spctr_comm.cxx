
/*Copyright (c) 2011, Edgar Solomonik, all rights reserved.*/

#include "../shared/util.h"
#include "spctr_comm.h"
#include "contraction.h"
#include "../tensor/untyped_tensor.h"

namespace CTF_int {
  spctr_replicate::spctr_replicate(contraction const * c,
                               int const *         phys_mapped,
                               int64_t             blk_sz_A,
                               int64_t             blk_sz_B,
                               int64_t             blk_sz_C)
       : spctr(c) {
    int i;
    int nphys_dim = c->A->topo->order;
    this->ncdt_A = 0;
    this->ncdt_B = 0;
    this->ncdt_C = 0;
    this->size_A = blk_sz_A;
    this->size_B = blk_sz_B;
    this->size_C = blk_sz_C;
    this->cdt_A = NULL;
    this->cdt_B = NULL;
    this->cdt_C = NULL;
    for (i=0; i<nphys_dim; i++){
      if (phys_mapped[3*i+0] == 0 &&
          phys_mapped[3*i+1] == 0 &&
          phys_mapped[3*i+2] == 0){
  /*      printf("ERROR: ALL-TENSOR REPLICATION NO LONGER DONE\n");
        ABORT;
        ASSERT(this->num_lyr == 1);
        hspctr->idx_lyr = A->topo->dim_comm[i].rank;
        hspctr->num_lyr = A->topo->dim_comm[i]->np;
        this->idx_lyr = A->topo->dim_comm[i].rank;
        this->num_lyr = A->topo->dim_comm[i]->np;*/
      } else {
        if (phys_mapped[3*i+0] == 0){
          this->ncdt_A++;
        }
        if (phys_mapped[3*i+1] == 0){
          this->ncdt_B++;
        }
        if (phys_mapped[3*i+2] == 0){
          this->ncdt_C++;
        }
      }
    }
    if (this->ncdt_A > 0)
      CTF_int::alloc_ptr(sizeof(CommData*)*this->ncdt_A, (void**)&this->cdt_A);
    if (this->ncdt_B > 0)
      CTF_int::alloc_ptr(sizeof(CommData*)*this->ncdt_B, (void**)&this->cdt_B);
    if (this->ncdt_C > 0)
      CTF_int::alloc_ptr(sizeof(CommData*)*this->ncdt_C, (void**)&this->cdt_C);
    this->ncdt_A = 0;
    this->ncdt_B = 0;
    this->ncdt_C = 0;
    for (i=0; i<nphys_dim; i++){
      if (!(phys_mapped[3*i+0] == 0 &&
          phys_mapped[3*i+1] == 0 &&
          phys_mapped[3*i+2] == 0)){
        if (phys_mapped[3*i+0] == 0){
          this->cdt_A[this->ncdt_A] = &c->A->topo->dim_comm[i];
        /*    if (is_used && this->cdt_A[this->ncdt_A].alive == 0)
            this->cdt_A[this->ncdt_A].activate(global_comm.cm);*/
          this->ncdt_A++;
        }
        if (phys_mapped[3*i+1] == 0){
          this->cdt_B[this->ncdt_B] = &c->B->topo->dim_comm[i];
  /*        if (is_used && this->cdt_B[this->ncdt_B].alive == 0)
            this->cdt_B[this->ncdt_B].activate(global_comm.cm);*/
          this->ncdt_B++;
        }
        if (phys_mapped[3*i+2] == 0){
          this->cdt_C[this->ncdt_C] = &c->C->topo->dim_comm[i];
  /*        if (is_used && this->cdt_C[this->ncdt_C].alive == 0)
            this->cdt_C[this->ncdt_C].activate(global_comm.cm);*/
          this->ncdt_C++;
        }
      }
    }
  }
 
  spctr_replicate::~spctr_replicate() {
    delete rec_ctr;
/*    for (int i=0; i<ncdt_A; i++){
      cdt_A[i]->deactivate();
    }*/
    if (ncdt_A > 0)
      CTF_int::cdealloc(cdt_A);
/*    for (int i=0; i<ncdt_B; i++){
      cdt_B[i]->deactivate();
    }*/
    if (ncdt_B > 0)
      CTF_int::cdealloc(cdt_B);
/*    for (int i=0; i<ncdt_C; i++){
      cdt_C[i]->deactivate();
    }*/
    if (ncdt_C > 0)
      CTF_int::cdealloc(cdt_C);
  }

  spctr_replicate::spctr_replicate(spctr * other) : spctr(other) {
    spctr_replicate * o = (spctr_replicate*)other;
    rec_ctr = o->rec_ctr->clone();
    size_A = o->size_A;
    size_B = o->size_B;
    size_C = o->size_C;
    ncdt_A = o->ncdt_A;
    ncdt_B = o->ncdt_B;
    ncdt_C = o->ncdt_C;
  }

  spctr * spctr_replicate::clone() {
    return new spctr_replicate(this);
  }

  void spctr_replicate::print() {
    int i;
    printf("spctr_replicate: \n");
    printf("cdt_A = %p, size_A = %ld, ncdt_A = %d\n",
            cdt_A, size_A, ncdt_A);
    for (i=0; i<ncdt_A; i++){
      printf("cdt_A[%d] length = %d\n",i,cdt_A[i]->np);
    }
    printf("cdt_B = %p, size_B = %ld, ncdt_B = %d\n",
            cdt_B, size_B, ncdt_B);
    for (i=0; i<ncdt_B; i++){
      printf("cdt_B[%d] length = %d\n",i,cdt_B[i]->np);
    }
    printf("cdt_C = %p, size_C = %ld, ncdt_C = %d\n",
            cdt_C, size_C, ncdt_C);
    for (i=0; i<ncdt_C; i++){
      printf("cdt_C[%d] length = %d\n",i,cdt_C[i]->np);
    }
    rec_ctr->print();
  }

  double spctr_replicate::est_time_fp(int nlyr){
    int i;
    double tot_sz;
    tot_sz = 0.0;
    for (i=0; i<ncdt_A; i++){
      ASSERT(cdt_A[i]->np > 0);
      tot_sz += cdt_A[i]->estimate_bcast_time(size_A*sr_A->el_size);
    }
    for (i=0; i<ncdt_B; i++){
      ASSERT(cdt_B[i]->np > 0);
      tot_sz += cdt_B[i]->estimate_bcast_time(size_B*sr_B->el_size);
    }
    for (i=0; i<ncdt_C; i++){
      ASSERT(cdt_C[i]->np > 0);
      tot_sz += cdt_C[i]->estimate_allred_time(size_C*sr_C->el_size);
    }
    return tot_sz;
  }

  double spctr_replicate::est_time_rec(int nlyr) {
    return rec_ctr->est_time_rec(nlyr) + est_time_fp(nlyr);
  }

  int64_t spctr_replicate::mem_fp(){
    return 0;
  }

  int64_t spctr_replicate::mem_rec(){
    return rec_ctr->mem_rec() + mem_fp();
  }


  void spctr_replicate::run(){
    int arank, brank, crank, i;

    arank = 0, brank = 0, crank = 0;
    for (i=0; i<ncdt_A; i++)
      arank += cdt_A[i]->rank;
    for (i=0; i<ncdt_B; i++)
      brank += cdt_B[i]->rank;
    for (i=0; i<ncdt_C; i++)
      crank += cdt_C[i]->rank;
    char * buf_A = this->A;
    char * buf_B = this->B;
    if (is_sparse_A){
      /*if (ncdt_A > 0){
        save_nnz_blk_A = (int64_t*)alloc(sizeof(int64_t)*nvirt_A);
        memcpy(save_nnz_blk_A,nnz_blk_A,sizeof(int64_t)*nvirt_A);
      }*/
      size_A = nnz_A;
      for (i=0; i<ncdt_A; i++){
        MPI_Bcast(&size_A, 1, MPI_INT64_T, 0, cdt_A[i]->cm);
        MPI_Bcast(nnz_blk_A, nvirt_A, MPI_INT64_T, 0, cdt_A[i]->cm);
      }
      MPI_Datatype md;
      bool need_free = get_mpi_dt(size_A, sr_A->pair_size(), md);
      
      if (nnz_A != size_A) 
        buf_A = (char*)alloc(sr_A->pair_size()*size_A);
      for (i=0; i<ncdt_A; i++){
        MPI_Bcast(buf_A, size_A, md, 0, cdt_A[i]->cm);
      }
      if (need_free) MPI_Type_free(&md);
    } else {
      for (i=0; i<ncdt_A; i++){
        MPI_Bcast(this->A, size_A, sr_A->mdtype(), 0, cdt_A[i]->cm);
      }
    }
    if (is_sparse_B){
      /*if (ncdt_B > 0){
        save_nnz_blk_B = (int64_t*)alloc(sizeof(int64_t)*nvirt_B);
        memcpy(save_nnz_blk_B,nnz_blk_B,sizeof(int64_t)*nvirt_B);
      }*/
      size_B = nnz_B;
      for (i=0; i<ncdt_B; i++){
        MPI_Bcast(&size_B, 1, MPI_INT64_T, 0, cdt_B[i]->cm);
        MPI_Bcast(nnz_blk_B, nvirt_B, MPI_INT64_T, 0, cdt_B[i]->cm);
      }
      MPI_Datatype md;
      bool need_free = get_mpi_dt(size_B, sr_B->pair_size(), md);
      
      if (nnz_B != size_B) 
        buf_B = (char*)alloc(sr_B->pair_size()*size_B);
      for (i=0; i<ncdt_B; i++){
        MPI_Bcast(buf_B, size_B, md, 0, cdt_B[i]->cm);
      }
      if (need_free) MPI_Type_free(&md);
    } else {
      for (i=0; i<ncdt_B; i++){
        MPI_Bcast(this->B, size_B, sr_B->mdtype(), 0, cdt_B[i]->cm);
      }
    }
    if (is_sparse_C){
      //FIXME: need to replicate nnz_blk_B for this
      assert(ncdt_C == 0);
    }
//    if (crank != 0) this->sr_C->set(this->C, this->sr_C->addid(), size_C);
    if (crank == 0 && !sr_C->isequal(this->beta, sr_C->mulid())){
      sr_C->scal(size_C, this->beta, this->C, 1);
/*      for (i=0; i<size_C; i++){
        sr_C->mul(this->beta, this->C+i*sr_C->el_size, this->C+i*sr_C->el_size);
      }*/
    }
//
    rec_ctr->set_nnz_blk_A(this->nnz_blk_A);
    rec_ctr->A         = buf_A;
    rec_ctr->nnz_A     = size_A;
    rec_ctr->set_nnz_blk_B(this->nnz_blk_B);
    rec_ctr->B         = buf_B;
    rec_ctr->nnz_B     = size_B;
    rec_ctr->C         = this->C;
    rec_ctr->nnz_blk_C = nnz_blk_C;
    if (crank != 0)
      rec_ctr->beta = sr_C->addid();
    else
      rec_ctr->beta = sr_C->mulid(); 
    rec_ctr->num_lyr      = this->num_lyr;
    rec_ctr->idx_lyr      = this->idx_lyr;

    rec_ctr->run();
    
    new_nnz_C = rec_ctr->new_nnz_C;
    new_C = rec_ctr->new_C;
    /*for (i=0; i<size_C; i++){
      printf("P%d C[%d]  = %lf\n",crank,i, ((double*)C)[i]);
    }*/
    for (i=0; i<ncdt_C; i++){
      ASSERT(!is_sparse_C);
      //ALLREDUCE(MPI_IN_PLACE, this->C, size_C, sr_C->mdtype(), sr_C->addmop(), cdt_C[i]->;
      MPI_Allreduce(MPI_IN_PLACE, this->C, size_C, sr_C->mdtype(), sr_C->addmop(), cdt_C[i]->cm);
    }

    if (is_sparse_A && buf_A != this->A) cdealloc(buf_A);
    if (is_sparse_B && buf_B != this->B) cdealloc(buf_B);
    if (!is_sparse_A && arank != 0){
      this->sr_A->set(this->A, this->sr_A->addid(), size_A);
    }
    if (!is_sparse_B && brank != 0){
      this->sr_B->set(this->B, this->sr_B->addid(), size_B);
    }
  }
}
