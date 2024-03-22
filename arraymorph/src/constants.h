#ifndef CONSTANTS
#define CONSTANTS

#include <vector>
using namespace std;


#define SUCCESS 1
#define FAIL -1
#define LOG_ENABLE
#define VOL_ENABLE
#define PROFILE_ENABLE
// #define PROFILE
#define FILL_VALUE 0
// #define DUMMY_WRITE
#define PROCESS
// #define MERGE_TEST
#define POOLEXECUTOR
const int s3Connections = 256;
const int gcConnections = 512;
const int gc_thread_num = 512;
const int azure_thread_num = 128;

const int requestTimeoutMs = 30000;
const int connectTimeoutMs = 30000;
const int poolSize = 8192;
const int num_per_merge = 1024;
enum FileFormat {
	binary=0,
	parquet,
	csv
};

typedef struct Result {
	size_t length;
	char* data;
} Result;

enum QPlan {
	NONE=-1,
	GET=0,
	GET_BYTE_RANGE=2,
	LAMBDA_MERGE=3,
	MERGE=4,
	LAMBDA=5
};

enum SPlan
{
	S3=0,
	GOOGLE,
	AZURE_BLOB
};

extern SPlan SP;
extern QPlan SINGLE_PLAN;
// micro-profiler

// throughput

const vector<vector<int>> REQUEST_RANGE{
	{32, 131072},
	{8, 8192},
	{8, 2048}
};

const vector<vector<int>> NUMBER_OF_REQUESTS{
	{8, 32, 128, 512, 2048, 8192, 32768, 131072},
	{8, 32, 128, 512, 2048, 8192},
	{8, 32, 128, 512, 2048, 8192}
};

const vector<vector<double>> THROUGHPUTS{
	{55.2, 75.6, 95.3, 95, 95, 107.4, 106.3, 99},
	{108.1, 108.7, 106, 106.7, 93, 75.8},
	{100.4, 107.4, 107.6, 104, 72, 59.9}
};

const vector<double> THROUGHPUTS_BY_SIZE{
	45, 70
};

// overhead

const vector<double> _alpha{57, 49, 55};
const vector<double> _beta{180, 72, 100};

const vector<double> _epsilon = {0.001, 0.001, 0.0295};

const vector<double> _gamma{0.0001, 0.97, 0.0005};

// price

// request

const vector<double> CR{0.0000004, 0.0000004, 0.0000005};

// data transfer (per GB)

// const vector<double> CT{0.05, 0.08, 0.05};
const vector<double> CT{0.09, 0.12, 0.08};
// lambda (per second)

const vector<double> CLAMBDA{0.0000169, 0.0000165, 0.000016};

// coefficient
const double phi = 105;




// chunking size in MB
const vector<vector<int>> CHUNK_SIZES{
	{2, 4, 8, 16, 32},
	{4, 8, 16},
	{4, 8, 16}
};  

#endif















