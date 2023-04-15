#ifndef QUERY_PROCESSOR
#define QUERY_PROCESSOR

#include "s3vl_chunk_obj.h"

// Cost
double Cost(S3VLChunkObj* chunk, QPlan qp, SPlan sp, double* lambda_t, double *cost);

// Band
double Band(SPlan sp, int nr);

// algorithm 2
vector<CPlan> QueryProcess(vector<S3VLChunkObj*> &chunks, SPlan sp, double *cost);



#endif