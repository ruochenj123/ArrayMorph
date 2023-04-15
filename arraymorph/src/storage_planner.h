#ifndef STORAGE_PLANNER
#define STORAGE_PLANNER

#include "s3vl_dataset_obj.h"
#include <bits/stdc++.h>
#include <cmath>
#include <float.h>


// algo 3
vector<vector<hsize_t>> Chunking(vector<hsize_t> dset_shape, int data_size,
	vector<vector<vector<hsize_t>>> &queries, SPlan sp);

// algo 4
vector<hsize_t> FinalDecision(vector<hsize_t> dset_shape, int data_size,
	vector<vector<vector<hsize_t>>> &queries, SPlan *SP);



#endif