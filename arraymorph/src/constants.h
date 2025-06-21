#ifndef CONSTANTS
#define CONSTANTS

#include <vector>
#include <string>


#define ARRAYMORPH_SUCCESS 1
#define ARRAYMORPH_FAIL -1
// #define LOG_ENABLE
#define PROFILE_ENABLE
#define FILL_VALUE 0
// #define DUMMY_WRITE
#define PROCESS
// #define POOLEXECUTOR

// #define MULTI_HYPERSLAB

const int THREAD_NUM = 256;
const int s3Connections = 256;
const int gcConnections = 512;


const int retries = 3;
const int requestTimeoutMs = 30000;
const int connectTimeoutMs = 30000;
const int poolSize = 128;

const int NUM_SPLITS = -1;
extern std::string BUCKET_NAME;

typedef struct Result {
	std::vector<char> data;
} Result;

enum QPlan {
	NONE=-1,
	GET=0,
	MERGE=1,
	LAMBDA=2,
	MULTI_FETCH=3,
	RANGE=4
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

const std::vector<std::vector<int>> REQUEST_RANGE{
	{32, 131072},
	{8, 2048},
	{8, 512}
};

const std::vector<std::vector<int>> NUMBER_OF_REQUESTS{
	{0, 8, 32, 128, 512, 2048, 8192, 32768, 131072, 524288},
	{0, 8, 32, 128, 512, 2048, 8192},
	{0, 8, 32, 128, 512, 2048, 8192}
};

const std::vector<std::vector<double>> THROUGHPUTS{
	{0, 100.8, 103.9, 104.4, 105, 105, 107.4, 106.3, 99, 34.3},
	{0, 108.1, 108.7, 106, 106.7, 93, 75.8},
	{0, 100.4, 107.4, 107.6, 104, 72, 59.9}
};

const std::vector<double> THROUGHPUTS_BY_SIZE{
	45, 70
};

// overhead

const std::vector<double> _alpha{15, 49, 55};
const std::vector<double> _beta{180, 72, 100};


const std::vector<double> _gamma{0.0001, 0.97, 0.0005};

// price

// request

const std::vector<double> CR{0.0000004, 0.0000004, 0.0000005};

// data transfer (per GB)

// const vector<double> CT{0.05, 0.08, 0.05};
const std::vector<double> CT{0.09, 0.12, 0.08};
// lambda (per second)

const std::vector<double> CLAMBDA{0.0000169, 0.0000165, 0.000016};
const std::vector<int> LAMBDA_MEM{1, 1, 2};
// coefficient
const double phi = 105;




// lambda restriction
const std::vector<int> LAMBDA_TIME_LIMIT {900, 540, 300};
const std::vector<int> LAMBDA_SIZE_LIMIT {1024 * 1024 * 1024, 10 * 1024 * 1024, 10 * 1024 * 1024};
const std::vector<double> LATENCY {0.0051, 0.001, 0.0295};

#endif















