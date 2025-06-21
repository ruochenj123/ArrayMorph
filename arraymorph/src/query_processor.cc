#include "query_processor.h"

// transfer size
// uint64_t TransferSize(S3VLChunkObj* chunk, QPlan qp) {
// 	if (qp == GET)
// 		return chunk->size;
// 	else if (qp == MERGE) {
// 		size_t start = 0, end = 0;
// 		for (int i = chunk->ndims - 1; i >= 0; i--) {
// 			start += chunk->reduc_per_dim[i] * chunk->ranges[i][0] * chunk->data_size;
// 			end += chunk->reduc_per_dim[i] * chunk->ranges[i][1] * chunk->data_size;
// 		}
// 		return (end - start + chunk->data_size);
// 	}
// 	else {
// 		return chunk->required_size;
// 	}
// }

// int CeilDiv(int a, int b) {
//     return (a + b - 1) / b;  // Ensures proper ceiling behavior
// }

// Cost of a single chunk
double Cost(const std::vector<std::unique_ptr<Segment>> & segments, QPlan plan, size_t num_requests, size_t chunk_size) {
	size_t total_transfer_size = 0;
	double throughput = Band(num_requests);
	int sp_idx = static_cast<int>(SP);
	double FP = 0.0;
	double TP = 0.0;
	size_t cur_num_requests;
	if (plan == LAMBDA) {
		for (auto &s : segments) 
			total_transfer_size += s->required_data_size;
		double lambda_exec_time = _alpha[sp_idx] * double(chunk_size) / 1024 / 1024 + _beta[sp_idx];
		FP += LAMBDA_MEM[sp_idx] * lambda_exec_time * CLAMBDA[sp_idx] / 1000;
		TP += lambda_exec_time / 1000.0 / THREAD_NUM;
		cur_num_requests = 1;
	}
	else {
		for (auto &s : segments)
			total_transfer_size += s->end_offset - s->start_offset + 1;
		cur_num_requests = segments.size();
	}
	FP += CT[sp_idx] * double(total_transfer_size) / 1024 / 1024 / 1024 + CR[sp_idx] * cur_num_requests;
	TP += double(total_transfer_size) / 1024 / 1024 / throughput + LATENCY[sp_idx] / THREAD_NUM * cur_num_requests;
	Logger::log("Throughput:",throughput,"TP:", TP, "FP", FP);
	return TP + phi * FP;
}

// Cost
// double Cost(std::vector<std::shared_ptr<S3VLChunkObj>> &chunks, int totalRequests, int lambdaRequests, SPlan sp, std::vector<CPlan> &plans) {
// 	uint64_t totalTransferSize = 0;
// 	int spIdx = static_cast<int>(sp);
// 	double FP = 0.0;
// 	double lambdaExecTime = _alpha[spIdx] * double(chunks[0]->size) / 1024 / 1024 + _beta[spIdx];
// 	for (auto &p : plans) {
// 		int chunk_idx = p.chunk_id;
// 		auto chunk = chunks[chunk_idx];
// 		uint64_t transferSize = TransferSize(chunk.get(), p.qp);
// 		int nr = 1;
// 		if (p.qp == GET_BYTE_RANGE) {
// 			for (int i = chunk->ndims - 2; i >= 0; i++)
// 				nr *= chunk->ranges[i][1] - chunk->ranges[i][0] + 1;
// 		}
// 		double fee = CR[spIdx] + CT[spIdx] * double(transferSize) / 1024 / 1024 / 1024;
// 		if (p.qp == LAMBDA) {
// 			int usedMem = 1;
// 			FP += usedMem * lambdaExecTime * CLAMBDA[spIdx] / 1000;
// 		}
// 		totalTransferSize += transferSize;
// 	}
// 	double throughput = Band(sp, totalRequests);
// 	Logger::log("# of requests:",totalRequests, "# of lambda requests:", lambdaRequests, "lambda execution time", lambdaExecTime);
// 	double TP = double(totalTransferSize) / 1024 / 1024 / throughput + LATENCY[spIdx] / CeilDiv(totalRequests, THREAD_NUM) + lambdaExecTime / 1000.0 * CeilDiv(lambdaRequests, THREAD_NUM);
// 	Logger::log("Throughput:",throughput,"TP:", TP, "FP", FP);
// 	return TP + phi * FP;
// }

// Band
double Band(int nt) {
    int pidx = static_cast<int>(SP);
    const std::vector<int>& requests = NUMBER_OF_REQUESTS[pidx];
    const std::vector<double>& throughputs = THROUGHPUTS[pidx];

    // Find the first element that is >= nt
    auto it = std::lower_bound(requests.begin(), requests.end(), nt);

    if (it == requests.begin()) {
        return throughputs.front();
    }

    if (it == requests.end()) {
        return throughputs.back();
    }

    int next_idx = std::distance(requests.begin(), it);
    int prev_idx = next_idx - 1;

    int N_i = requests[prev_idx];
    int N_j = requests[next_idx];
    double B_i = throughputs[prev_idx];
    double B_j = throughputs[next_idx];

    // Perform linear interpolation
    double B_interp = B_i + ((B_j - B_i) * (nt - N_i) / (N_j - N_i));

    return B_interp;
}



// check Lambda
bool CheckLambda(size_t transfer_size, size_t chunk_size) {
	int idx = static_cast<int>(SP);
	double lambda_exec_time = _alpha[idx] * double(chunk_size) / 1024 / 1024 + _beta[idx];
	return (transfer_size <= LAMBDA_SIZE_LIMIT[idx]) && (lambda_exec_time <= LAMBDA_TIME_LIMIT[idx] * 1000);
}


std::vector<CPlan> QueryProcess(std::vector<std::vector<std::unique_ptr<Segment>>> &&segments, std::vector<std::shared_ptr<S3VLChunkObj>>&chunks, std::vector<std::list<std::vector<hsize_t>>> &global_mapping) {
	assert(segments.size() == chunks.size());
	size_t num_requests = chunks.size();
	std::vector<CPlan> plans;
	for (int i = 0; i < chunks.size(); i++) {
		int plan_request;
		if (SINGLE_PLAN != NONE) {
			plans.emplace_back(i, SINGLE_PLAN, segments[i].size(), std::move(segments[i]));
		}
		else {
			auto & chunk_mapping = global_mapping[i];
			Logger::log("Splitting chunk ", i);
			auto range_segments = SplitAndProcess(std::move(segments[i]),chunk_mapping, num_requests, chunks[i]->size);
			size_t required_size = 0;
			for (auto &s : range_segments)
				required_size += s->required_data_size;
			double range_cost  =Cost(range_segments, RANGE, num_requests + range_segments.size() - 1, chunks[i]->size);
			double lambda_cost = Cost(range_segments, LAMBDA, num_requests, chunks[i]->size);
			// std::cout << range_cost << " " << lambda_cost << " " << CheckLambda(required_size, chunks[i]->size) << std::endl;
			Logger::log("Range Cost: ", range_cost, "Lambda Cost: ", lambda_cost);
			if (CheckLambda(required_size, chunks[i]->size) == false || range_cost < lambda_cost) {
				plans.emplace_back(i, RANGE, range_segments.size(), std::move(range_segments));
				num_requests += range_segments.size() - 1;
			}
			else { // Use Lambda
				std::vector<std::unique_ptr<Segment>> seg_list;
				seg_list.emplace_back(std::make_unique<Segment>(0, chunks[i]->size - 1, required_size, chunk_mapping.begin(), chunk_mapping.end(), chunk_mapping.size()));
				plans.emplace_back(i, LAMBDA, 1, std::move(seg_list));
			}
		}
		Logger::log("Final plan: ", plans.back().qp);
#ifdef LOG_ENABLE
		for (const auto &s :plans.back().segments)
			Logger::log(s->to_string());
#endif
	}
	return plans;
}

std::vector<std::unique_ptr<Segment>> SplitSingleSegment(const std::unique_ptr<Segment> &s, std::list<std::vector<hsize_t>> &global_mapping) {
	std::vector<std::unique_ptr<Segment>> res;
	auto mapping_start = s->mapping_start;
	auto mapping_end = s->mapping_end;
	if (std::prev(mapping_end) != mapping_start) { // multiple mapping
		size_t mid_size = s->mapping_size / 2;
		auto mid_it = mapping_start;
		std::advance(mid_it, mid_size);
		res.emplace_back(std::make_unique<Segment>(mapping_start, mid_it, mid_size));
		res.emplace_back(std::make_unique<Segment>(mid_it, mapping_end, s->mapping_size - mid_size));
	}
	else {
		auto &m = *mapping_start;
		hsize_t left_size = m[2] / 2;
		hsize_t right_size = m[2] - left_size;
		m[2] = left_size;

		std::vector<hsize_t> new_mapping = {m[0] + left_size, m[1] + left_size, right_size};
		auto new_it = global_mapping.insert(std::next(mapping_start), new_mapping);
		res.emplace_back(std::make_unique<Segment>(mapping_start, new_it, 1));
		res.emplace_back(std::make_unique<Segment>(new_it, std::next(new_it), 1));		
	}
	return res;
}
// Merge two segments, s1 < s2 by start offset
// std::unique_ptr<Segment> Merge(const std::unique_ptr<Segment> &s1, const std::unique_ptr<Segment> &s2) {
// 	assert(s1->end_offset <= s2->start_offset);
// 	std::vector<std::vector<hsize_t>> mapping;
// 	mapping.reserve(s1->mapping.size() + s2->mapping.size());
// 	mapping.insert(mapping.end(), s1->mapping.begin(), s1->mapping.end());
// 	mapping.insert(mapping.end(), s2->mapping.begin(), s2->mapping.end());
// 	return std::make_unique<Segment>(s1->start_offset, s2->end_offset, s1->required_data_size + s2->required_data_size, mapping);
// }

std::vector<std::unique_ptr<Segment>> MergeAll(const std::vector<std::unique_ptr<Segment>> &segments, int start_idx, int end_idx) {
	std::vector<std::unique_ptr<Segment>> res_list;
	if (start_idx + 1 == end_idx) {
		const auto & s = segments[start_idx];
		res_list.emplace_back(std::make_unique<Segment>(s->start_offset, s->end_offset, s->required_data_size, s->mapping_start, s->mapping_end, s->mapping_size));
	}
	else {
		const auto &first = segments[start_idx];
		const auto &last = segments[end_idx - 1];
		size_t required_size = 0;
		size_t mapping_size = 0;
		for (int i = start_idx; i < end_idx; i++) {
			required_size += segments[i]->required_data_size;
			mapping_size += segments[i]->mapping_size;
		}
		res_list.emplace_back(std::make_unique<Segment>(first->start_offset, last->end_offset, required_size, first->mapping_start, last->mapping_end, mapping_size));
	}
	return res_list;
}


std::vector<std::unique_ptr<Segment>> SplitAndProcess(std::vector<std::unique_ptr<Segment>> &&segments, std::list<std::vector<hsize_t>> &global_mapping, size_t num_requests, size_t chunk_size) {
	Logger::log("start splitting");
	std::vector<std::vector<std::unique_ptr<Segment>>> seg_lists;
	seg_lists.push_back(std::move(segments));
	std::queue<std::vector<size_t>> q; // store the segment list id, start_idx, and end_idx of segments
	std::vector<std::unique_ptr<Segment>> res;
	q.push({0, 0, seg_lists[0].size()});
	int num_split = 0;
	while(!q.empty() && ( NUM_SPLITS == -1 || (num_split < NUM_SPLITS))) {
		auto cur_range = q.front();
		size_t seg_idx = cur_range[0]; 			// current segment list to split
		size_t start_idx = cur_range[1];		// start index of segment for the current segment list
		size_t end_idx = cur_range[2];			// end index of segment for the current segment list
		q.pop();
		if (start_idx + 1 == end_idx) { // single segment
			std::vector<std::unique_ptr<Segment>> split_list = SplitSingleSegment(seg_lists[seg_idx][start_idx], global_mapping);
			seg_lists.push_back(std::move(split_list));
			q.push({seg_lists.size() - 1, 0, 2});
			continue;
		}
		// check the max gap
		size_t max_gap_idx;
		std::vector<size_t> candidates;
#ifdef MULTI_HYPERSLAB
		size_t max_gap = 0;
		for (size_t i = start_idx; i < end_idx - 1; i++) {
			size_t gap = seg_lists[seg_idx][i + 1]->start_offset - seg_lists[seg_idx][i]->end_offset - 1;
			if (gap == max_gap) {
				candidates.push_back(i + 1);
			}
			else if (gap > max_gap) {
				max_gap = gap;
				candidates = {i + 1};
			}
		}
		max_gap_idx = candidates[candidates.size() / 2];
#else
		max_gap_idx = (start_idx + end_idx) / 2;
#endif
		// check if do split
		auto merge_all = MergeAll(seg_lists[seg_idx], start_idx, end_idx);
		auto merge_left = MergeAll(seg_lists[seg_idx], start_idx, max_gap_idx);
		auto merge_right = MergeAll(seg_lists[seg_idx], max_gap_idx, end_idx);
		double split_cost = Cost(merge_left, RANGE, num_requests + 1, chunk_size) + Cost(merge_right, RANGE, num_requests + 1, chunk_size);
		double merge_cost = Cost(merge_all, RANGE, num_requests, chunk_size);
		Logger::log("Split cost: ", split_cost, "Merge cost: ", merge_cost);
		if (// merge_all[0]->end_offset - merge_all[0]->start_offset + 1 >= 1024 * 1024 && 
			split_cost < merge_cost) {
			q.push({seg_idx, start_idx, max_gap_idx});
			q.push({seg_idx, max_gap_idx, end_idx});
			num_requests++;
			num_split++;
		}
		else {
			res.push_back(std::move(merge_all.back()));
		}
	}
	while (!q.empty()) {
		auto cur_range = q.front();
		q.pop();
		auto merge_all = MergeAll(seg_lists[cur_range[0]], cur_range[1], cur_range[2]);
		res.push_back(std::move(merge_all.back()));
	}
	Logger::log("Split result: ");
#ifdef LOG_ENABLE
	for (const auto &s :res)
		Logger::log(s->to_string());
#endif
	return res;

}

// algo 2
// std::vector<CPlan> QueryProcess(std::vector<std::shared_ptr<S3VLChunkObj>> &chunks, SPlan sp, double *cost) {
// 	int nr = chunks.size();
// 	int nlambda = 0;
// 	int ndims = chunks[0]->ndims;
// 	std::vector<CPlan> plans(nr);
// 	if (SINGLE_PLAN != NONE) {
// 		for (int i = 0; i < nr; i++) {
// 			// std::cout << "required size: " << chunks[i]->required_size << std::endl;
// 			// std::cout << "chunk size: " << chunks[i]->size << std::endl;
// 			plans[i] = CPlan{i, SINGLE_PLAN};
// 		}
// 		return plans;
// 	}
// 	// optimization
// 	std::vector<std::pair<uint64_t, CPlan>> pending;
// 	for (int i = 0; i < nr; i++) {
// 		auto chunk = chunks[i];
// 		plans[i] = {i, GET};
// 		// Logger::log("------ plan for chunk: ", chunk->uri);
// 		if (chunk->size > chunk->required_size) {
// 			uint64_t mergeSize = TransferSize(chunk.get(), MERGE);
// 			Logger::log("MergeSize:", mergeSize, "ChunkSize:", chunk->size, "RequestSize:", chunk->required_size);
// 			if (mergeSize == chunk->required_size || !CheckLambda(chunk.get(), sp))
// 				plans[i].qp = MERGE;
// 			else {
// 				pending.push_back({mergeSize - chunk->required_size, plans[i]});
// 			}
// 		}
// 	}
// 	std::sort(pending.begin(), pending.end(), [](const std::pair<uint64_t, CPlan>& a, const std::pair<uint64_t, CPlan>& b) {
//         return a.first > b.first;  // Compare first elements, in decreasing order
//     });

// 	int pn = pending.size();
// 	int idx = 0;
// 	while (idx <= pn) {
// 		std::vector<CPlan> lambdaPlans, mergePlans;
// 		for (int i = idx; i < std::min(idx + THREAD_NUM, pn); i++) {
// 			lambdaPlans.push_back(CPlan{pending[i].second.chunk_id, LAMBDA});
// 			mergePlans.push_back(CPlan{pending[i].second.chunk_id, MERGE});
// 		}
// 		double lambdaCost = Cost(chunks,lambdaPlans.size(), lambdaPlans.size(), sp, lambdaPlans);
// 		double mergeCost = Cost(chunks, mergePlans.size(), 0, sp, mergePlans);
// 		Logger::log("lambdaCost:",lambdaCost, "mergeCost:",mergeCost);
// 		if (lambdaCost < mergeCost) {
// 			for (int i = idx; i < std::min(idx + THREAD_NUM, pn); i++) {
// 				int chunkId = pending[i].second.chunk_id;
// 				plans[chunkId].qp = LAMBDA;
// 				nlambda++;
// 			}
// 		}
// 		else {
// 			for (int i = idx; i < pn; i++) {
// 				int chunkId = pending[i].second.chunk_id;
// 				plans[chunkId].qp = MERGE;
// 			}
// 			break;
// 		}
// 	}
// 	double totalCost = Cost(chunks, nr, nlambda, sp, plans);
// 	// adjustment
// 	if (nr < REQUEST_RANGE[static_cast<int>(sp)][0]) {
// 		std::vector<CPlan> new_plans(plans.begin(), plans.end());

// 		std::vector<std::pair<int, size_t>> tmp(new_plans.size());
// 		for (auto &p : new_plans) {
// 			int chunk_id = p.chunk_id;
// 			tmp[chunk_id] = {chunk_id, chunks[chunk_id]->ranges[ndims - 1][1] - chunks[chunk_id]->ranges[ndims - 1][0] + 1};
// 		}
// 		std::sort(tmp.begin(), tmp.end(), [](const std::pair<int, size_t>& a, const std::pair<int, size_t>& b) {
// 			return a.second < b.second;
// 		});

// 		while (tmp.size() > 0 && nr < REQUEST_RANGE[sp][0]) {
// 			auto cur = tmp.back();
// 			tmp.pop_back();
// 			int chunk_id = cur.first;
// 			auto chunk = chunks[chunk_id];
// 			assert (new_plans[chunk_id].chunk_id == chunk_id);
// 			if (new_plans[chunk_id].qp == LAMBDA)
// 				nlambda--;
// 			new_plans[chunk_id].qp = GET_BYTE_RANGE;
// 			int cur_nr = 1;
// 			for (int i = ndims - 2; i >= 0 ; i--)
// 				cur_nr *= chunk->ranges[i][1] - chunk->ranges[i][0] + 1;
// 			nr = nr - 1 + cur_nr;
// 		}
// 		double newTotalCost = Cost(chunks, nr, nlambda, sp, plans);
// 		Logger::log("new cost: ", newTotalCost);
// 		Logger::log("old cost: ", totalCost);
// 		if (newTotalCost < totalCost) {
// 			totalCost = newTotalCost;
// 			plans = new_plans;
// 		}
// 	}
// 	*cost = totalCost;
// 	for (int i = 0; i < plans.size(); i++)
// 		assert(i == plans[i].chunk_id);
// 	return plans;
// }