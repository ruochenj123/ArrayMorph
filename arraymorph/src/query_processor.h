#ifndef QUERY_PROCESSOR
#define QUERY_PROCESSOR

#include "s3vl_chunk_obj.h"
#include "utils.h"

// algorithm 2
// std::vector<CPlan> QueryProcess(std::vector<std::shared_ptr<S3VLChunkObj>>&chunks, SPlan sp, double *cost);

double Band(int nt);
bool CheckLambda(size_t transfer_size, size_t chunk_size);
std::vector<std::unique_ptr<Segment>> SplitSingleSegment(const std::unique_ptr<Segment> & s);
// std::unique_ptr<Segment> Merge(const std::unique_ptr<Segment> &s1, const std::unique_ptr<Segment> &s2);
std::vector<std::unique_ptr<Segment>> MergeAll(const std::vector<std::unique_ptr<Segment>> &segments, int start_idx, int end_idx);
std::vector<std::unique_ptr<Segment>> SplitAndProcess(std::vector<std::unique_ptr<Segment>> &&segments, std::list<std::vector<hsize_t>> &global_mapping, size_t num_requests, size_t chunk_size);

double Cost(const std::vector<std::unique_ptr<Segment>> & segments, QPlan plan, size_t num_requests, size_t chunk_size);
std::vector<CPlan> QueryProcess(std::vector<std::vector<std::unique_ptr<Segment>>> &&segments, std::vector<std::shared_ptr<S3VLChunkObj>>&chunks, std::vector<std::list<std::vector<hsize_t>>> &global_mapping);

#endif