#ifndef OPERATORS
#define OPERATORS
#include <aws/core/Aws.h>
#include <aws/core/utils/logging/LogLevel.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/SelectObjectContentRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/CSVInput.h>                          // for CSVInput
#include <aws/s3/model/CSVOutput.h>                         // for CSVOutput
#include <aws/s3/model/ExpressionType.h>                    // for Expressio...
#include <aws/s3/model/FileHeaderInfo.h>                    // for FileHeade...
#include <aws/s3/model/InputSerialization.h>                // for InputSeri...
#include <aws/s3/model/OutputSerialization.h>               // for OutputSer...
#include <aws/s3/model/RecordsEvent.h>                      // for RecordsEvent
#include <aws/s3/model/SelectObjectContentHandler.h>        // for SelectObj...
#include <aws/s3/model/StatsEvent.h>                        // for StatsEvent
#include <aws/core/http/Scheme.h>                           // for Scheme
#include <aws/core/client/ClientConfiguration.h>
#include <azure/storage/blobs.hpp>                          // for Azure blob
#include "google/cloud/storage/client.h"                    // for GC
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <utility>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <hdf5.h>
#include <mutex>
#include <chrono>
#include <curl/curl.h>
#include "constants.h"
#include "logger.h"
#include "profiler.h"
using namespace std;
using namespace Aws;
using namespace Aws::Auth;
using namespace Aws::Http;
using namespace Aws::Client;
using namespace Aws::S3;
using namespace Aws::S3::Model;
using namespace Azure::Storage::Blobs;
namespace gcs = ::google::cloud::storage;

extern Profiler S3profiler;

extern atomic<int> finish;

class AsyncWriteInput:public AsyncCallerContext {
public:
	AsyncWriteInput(const char* buf): buf(buf){}
	const char* buf;
};

class AsyncReadInput: public AsyncCallerContext {
public:
	AsyncReadInput(const void* buf, const vector<vector<hsize_t>> &mapping, const int lambda=0)
		:buf(buf), mapping(mapping), lambda(lambda){}
	const void* buf;
	const vector<vector<hsize_t>> mapping;
	const int lambda;
};

struct response {
  char *memory;
  size_t size;
};

class Operators {
public:
// S3
	static Result S3Get(S3Client *client, string bucket_name, const Aws::String &object_name);
	static herr_t S3GetAsync(S3Client *client, string bucket_name, const Aws::String &object_name, std::shared_ptr<AsyncCallerContext> input);
	static herr_t S3GetByteRangeAsync(S3Client *client, string bucket_name, const Aws::String &object_name, 
		uint64_t beg, uint64_t end, std::shared_ptr<AsyncCallerContext> input);
	// Result S3select(const Query &q);
	// Result S3select(const Aws::String &object_name, string expr, FileFormat format);

    // bool S3put(const Aws::String &object_name, const string& file_path);
    static herr_t S3Put(S3Client *client, string bucket_name, string object_name, Result &re);
    static herr_t S3PutBuf(S3Client *client, string bucket_name, string object_name, char* buf, hsize_t length);
    // static herr_t S3Put(S3Client *client, string bucket_name, const Aws::String &object_name, string content);
    static herr_t S3PutAsync(S3Client *client, string bucket_name, const Aws::String &object_name, Result &re);
    static herr_t S3Delete(const S3Client *client, string bucket_name, const Aws::String &object_name);
    // bool S3createBucket(const BucketLocationConstraint &region = BucketLocationConstraint::us_east_1);
    // char* processOneRead(Query q);
    static void GetAsyncCallback(const Aws::S3::S3Client* s3Client, const Aws::S3::Model::GetObjectRequest& request, Aws::S3::Model::GetObjectOutcome outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context);

//Azure
    static herr_t AzurePut(BlobContainerClient &client, string blob_name, uint8_t *buf, size_t length);
    static herr_t AzureGet(BlobContainerClient &client, string blob_name, shared_ptr<AsyncCallerContext> context);
    static herr_t AzureGetRange(BlobContainerClient &client, string blob_name, uint64_t beg, uint64_t end, shared_ptr<AsyncCallerContext> context);

// Google Cloud
    static herr_t GCPut(gcs::Client *client, string bucket_name, string obj_name, char* buf, hsize_t length);
    static herr_t GCGet(gcs::Client *client, string bucket_name, string obj_name, shared_ptr<AsyncCallerContext> context);
    static herr_t GCGetRange(gcs::Client *client, string bucket_name, string obj_name, uint64_t beg, uint64_t end, shared_ptr<AsyncCallerContext> context);
// Azure and GC lambda
    static herr_t HttpTrigger(string lambda_url, string query, shared_ptr<AsyncCallerContext> context);
};

inline herr_t Operators::S3GetByteRangeAsync(S3Client *client, string bucket_name, const Aws::String &object_name, uint64_t beg, uint64_t end,
                                std::shared_ptr<AsyncCallerContext> input)
{
    Logger::log("------ S3getRangeAsync ", object_name);
    GetObjectRequest request;
    request.SetBucket(bucket_name);
    request.SetKey(object_name);
    stringstream ss;
    ss << "bytes=" << beg << '-' << end;
    request.SetRange(ss.str().c_str());
    client->GetObjectAsync(request, GetAsyncCallback, input);
    return SUCCESS;
}



#endif
