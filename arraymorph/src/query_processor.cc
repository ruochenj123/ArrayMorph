#include "query_processor.h"
// Cost
double Cost(S3VLChunkObj* chunk, QPlan qp, SPlan sp, int nt, double* lambda_t) {
	size_t nr, st;
	double c, to;
	int idx = static_cast<int>(sp);
	if (qp == GET) {
		nr = 1;
		st = chunk->size;
		c = CR[idx] + CT[idx] * double(st) / 1024 / 1024 / 1024;
		to = 0;
	}
	else if (qp == GET_BYTE_RANGE) {
		nr = 1;
		for (int i = 1; i < chunk->ndims; i++)
			nr *= chunk->ranges[i][1] - chunk->ranges[i][0] + 1;
		st = chunk->required_size;
		c = CR[idx] * nr + CT[idx] * double(st) /1024 / 1024 / 1024;
		to = nr * _epsilon[idx] + 10000;
	}
	else if (qp == MERGE) {
		nr = 1;
		size_t start = 0, end = 0;
		for (int i = chunk->ndims - 1; i >= 0; i--) {
			start += chunk->reduc_per_dim[i] * chunk->ranges[i][0] * chunk->data_size;
			end += chunk->reduc_per_dim[i] * chunk->ranges[i][1] * chunk->data_size;
		}
		st = end - start + 1;
		c = CR[idx] + CT[idx] * double(st) / 1024 / 1024 / 1024;
		to = _epsilon[idx];
	}
	else {
		double lambda_exec_time = _alpha[idx] * double(chunk->size) / 1024 / 1024 + _beta[idx];
		nr = 1;
		st = chunk->required_size;
		double used_mem = 1;
		if (sp == SPlan::AZURE_BLOB)
			used_mem = chunk->size / 1024 / 1024 / 1024;
		c = CR[idx] + CT[idx] * double(st) / 1024 / 1024 / 1024 + used_mem * lambda_exec_time * CLAMBDA[idx] / 1000;
		to = lambda_exec_time * _gamma[idx] / 1000;
		*lambda_t = lambda_exec_time;
	}
	double throughput;
	if (sp == SPlan::S3 || chunk->size >= 4 * 1024 *1024)
		throughput = Band(sp, nt);
	else{
		throughput = THROUGHPUTS_BY_SIZE[chunk->size / 1024 / 1024];
	}
	double time = double(st) / 1024 / 1024 / throughput + to;
	double cost = time + phi * c;
	// double cost = c;
	Logger::log("throughput: ", throughput);
	Logger::log("time: ", time);
	Logger::log("cost: ", c);
	return cost;
}

// Band
double Band(SPlan sp, int nt) {
	int idx = static_cast<int>(sp);
	const vector<int> requests = NUMBER_OF_REQUESTS[idx];
	const vector<double> throughputs = THROUGHPUTS[idx];
	int range = requests.size();
	assert(range == throughputs.size());
	int i;
	for (i = 0; i < range; i++) {
		if (requests[i] > nt)
			break;
	}
	Logger::log("------ Bandwidth estimator");
	Logger::log("SPlan: ", sp);
	Logger::log("# of requests:", nt);
	Logger::log("idx: ", i);
	if (i == 0)
		return throughputs[i];
	if (nt < requests[i] / 2)
		return throughputs[i - 1];
	return throughputs[i];
}

bool Cmp(vector<size_t> &a, vector<size_t> &b) {
	return a[1] < b[1];
}

// algo 2
vector<CPlan> QueryProcess(vector<S3VLChunkObj*> &chunks, SPlan sp, double *cost) {
	int nr = chunks.size();
	vector<CPlan> plans(nr);
	if (SINGLE_PLAN != NONE) {
		for (int i = 0; i < nr; i++) {
			plans[i] = CPlan{i, SINGLE_PLAN};
		}
		return plans;
	}
	double total_cost = 0;
	double lambda_t;
	for (int i = 0; i < nr; i++) {
		S3VLChunkObj *chunk = chunks[i];
		CPlan p{i, GET};
		Logger::log("------ plan for chunk: ", chunk->uri);
		if (chunk->size > chunk->required_size) {
			double c = Cost(chunk, GET, sp, nr, &lambda_t);
			double c1 = Cost(chunk, MERGE, sp, nr, &lambda_t);
			if (c1 < c) {
				c = c1;
				p.qp = MERGE;
			}
			if (sp == SPlan::S3) {
				c1 = Cost(chunk, LAMBDA, sp, nr, &lambda_t);
				Logger::log("S3 Lambda Time: ", lambda_t);
				Logger::log("S3 Lambda Cost: ", c1);
				if (lambda_t < 60000 && c1 < c) {
					p.qp = LAMBDA;
					c = c1;
				}
			}
			else if (sp == SPlan::GOOGLE && chunk->required_size < 10 * 1024 * 1024) {
				c1 = Cost(chunk, LAMBDA, sp, nr, &lambda_t);
				Logger::log("GC Lambda Time: ", lambda_t);
				Logger::log("GC Lambda Cost: ", c1);
				if (c1 < c) {
					p.qp = LAMBDA; 
					c = c1;
				}
			}
			else if (chunk->required_size < 5 * 1024 * 1024) {
				c1 = Cost(chunk, LAMBDA, sp, nr, &lambda_t);
				Logger::log("Azure Lambda Time: ", lambda_t);
				Logger::log("Azure Lambda Cost: ", c1);
				if (c1 < c) {
					p.qp = LAMBDA; 
					c = c1;
				}	
			}
			total_cost += c;
		}
		else
			total_cost += Cost(chunk, GET, sp, nr, &lambda_t);
		plans[i] = p;
	}
	// resizing
	if (nr < REQUEST_RANGE[sp][0]) {
		Logger::log("resizing");
		vector<CPlan> new_plans(plans.begin(), plans.end());

		vector<vector<size_t>> tmp(nr);
		for (int i = 0; i < nr; i++)
			tmp[i] = {i, chunks[i]->ranges[0][1] - chunks[i]->ranges[0][0] + 1};
		sort(tmp.begin(), tmp.end(), Cmp);

		while (tmp.size() > 0 && nr < REQUEST_RANGE[sp][0]) {
			vector<size_t> cur = tmp.back();
			tmp.pop_back();
			S3VLChunkObj *chunk = chunks[cur[0]];
			new_plans[cur[0]].qp = GET_BYTE_RANGE;
			int cur_nr = 1;
			for (int i = 1; i < chunk->ndims; i++)
				cur_nr *= chunk->ranges[i][1] - chunk->ranges[i][0] + 1;
			nr = nr - 1 + cur_nr;
		}
		double new_cost = 0;
		for (int i = 0; i < chunks.size(); i++) {
			new_cost += Cost(chunks[i], new_plans[i].qp, sp, nr, &lambda_t);
		}
		Logger::log("new cost: ", new_cost);
		Logger::log("old cost: ", total_cost);
		if (new_cost < total_cost) {
			total_cost = new_cost;
			plans = new_plans;
		}
	}
	*cost = total_cost;
	return plans;

}









