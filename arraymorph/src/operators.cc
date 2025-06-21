#include "operators.h"
#include "logger.h"
#include <assert.h>
#include <time.h>
#include <chrono>


CloudClient global_cloud_client;


OperationTracker& OperationTracker::getInstance() {
    static OperationTracker instance;
    return instance;
}

void OperationTracker::add() {
    finish.fetch_add(1, std::memory_order_relaxed);
}

int OperationTracker::get() const {
    return finish.load(std::memory_order_relaxed);
}

void OperationTracker::reset() {
    finish.store(0, std::memory_order_relaxed);
}

void PutAsyncCallback(const Aws::S3::S3Client* s3Client, 
    const Aws::S3::Model::PutObjectRequest& request, 
    const Aws::S3::Model::PutObjectOutcome& outcome,
    const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) {
    if (outcome.IsSuccess()) {
        Logger::log("write async successfully: ", request.GetKey());
        OperationTracker::getInstance().add();
    }
    else {
        Logger::log("write async failed: ", request.GetKey());
    }
    const std::shared_ptr<const AsyncWriteInput> input = std::static_pointer_cast<const AsyncWriteInput>(context);
    delete[] input->buf;
}


void Operators::GetAsyncCallback(const Aws::S3::S3Client* s3Client, 
    const Aws::S3::Model::GetObjectRequest& request, 
    Aws::S3::Model::GetObjectOutcome outcome,
    const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) {
    const std::shared_ptr<const AsyncReadInput> input = std::static_pointer_cast<const AsyncReadInput>(context);
    if (outcome.IsSuccess()) {
        Logger::log("read async successfully: ", request.GetKey());
        auto& file = outcome.GetResultWithOwnership().GetBody();
        file.seekg(0, file.end);
        size_t length = file.tellg();
        file.seekg(0, file.beg);
      
#ifdef PROCESS
        if (length < 1024 * 1024 * 1024) {
            char* buf = new char[length];
            file.read(buf, length);
            // std::stringstream ss;
            // ss << "object length: " << length << std::endl;
            for (auto &m: input->mapping) {
                memcpy((char*)input->buf + m[1], buf + m[0], m[2]);
                // ss << m[0] << " " << m[1] << " " << m[2] << std::endl;
            }
            // std::cout << ss.str();
            delete[] buf;
        }
        else {
            for (auto &m: input->mapping) {
                file.seekg(m[0], file.beg);
                file.read((char*)input->buf + m[1], m[2]);
            }
        }
#endif
    OperationTracker::getInstance().add();
    } else {
        auto err = outcome.GetError();
        std::cerr << request.GetKey() << std::endl;
        std::cerr << "Error: GetObject: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
        if (input->lambda == 1) {
            S3GetAsync(s3Client, input->bucket_name, input->uri, context);
            Logger::log("Lambda fails, retry on GET");
        }
#ifndef PROCESS
        // for profiling
        OperationTracker::getInstance().add();
#endif
    }
    Logger::log("process async successfully: ", request.GetKey());
}


herr_t Operators::S3GetAsync(const S3Client *client, const std::string& bucket_name, const Aws::String &object_name,
                    const std::shared_ptr<const AsyncCallerContext> input)
{
    Logger::log("------ S3getAsync ", object_name);
    Logger::log("------ S3getAsync ", bucket_name);
    GetObjectRequest request;
    request.SetBucket(bucket_name);
    request.SetKey(object_name);
    // auto handler = [&](const HttpRequest* http_req,
    //                     HttpResponse* http_resp,
    //                     long long) {
    //     if (http_resp->HasHeader("triggered"))
    //         return;
    //     http_resp->AddHeader("triggered", "Yes");
    //     auto headers = http_resp->GetHeaders();
    //     cout << "rid: " << headers["x-amz-request-id"] << endl;
    // };
    // request.SetDataReceivedEventHandler(std::move(handler));
    // std::cout << "send lambda" << std::endl;
    client->GetObjectAsync(request, GetAsyncCallback, input);
    // client->GetObject(request);
    // std::cout << "finish lambda" << std::endl;
    return ARRAYMORPH_SUCCESS;
}

Result Operators::S3Get(const S3Client *client, const std::string& bucket_name, const Aws::String &object_name)
{
    Result re;


    Logger::log("------ S3get ", object_name);
    Logger::log("bucket_name ", bucket_name);
    GetObjectRequest request;
    request.SetBucket(bucket_name);
    request.SetKey(object_name);

    auto outcome = client->GetObject(request);
    if (outcome.IsSuccess()) {
        auto& file = outcome.GetResultWithOwnership().GetBody();
        file.seekg(0, file.end);
        size_t length = file.tellg();
        file.seekg(0, file.beg);
        re.data.resize(length);
        file.read(re.data.data(), length);
    } else {
        auto err = outcome.GetError();
        std::cerr << "Error: GetObject: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    }
    return re;
}

herr_t Operators::S3Delete(const S3Client *client, const std::string& bucket_name, const Aws::String &object_name) {
    Logger::log("------ S3Delete ", object_name);
    DeleteObjectRequest request;
    request.SetBucket(bucket_name);
    request.SetKey(object_name);

    auto outcome = client->DeleteObject(request);
    if (!outcome.IsSuccess())
    {
        auto err = outcome.GetError();
        std::cerr << "Error: DeleteObject: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
        return ARRAYMORPH_FAIL;
    }
    return ARRAYMORPH_SUCCESS;
}
// herr_t Operators::S3Put(S3Client *client, string bucket_name, const Aws::String &object_name, string content)
// {
//     Logger::log("------ S3Put ", object_name);
//     PutObjectRequest request;
//     request.SetBucket(bucket_name);
//     request.SetKey(object_name);
    
//     auto input_data = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream", std::stringstream::in | std::stringstream::out);
//     *input_data << content;
//     request.SetBody(input_data);

//     auto outcome = client->PutObject(request);
//     if (!outcome.IsSuccess()) {
//         auto err = outcome.GetError();
//         std::cout << "ERROR: PutObject: " << 
//             err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
//         return ARRAYMORPH_FAIL;
//     }
//     return ARRAYMORPH_SUCCESS;
// }

herr_t Operators::S3PutBuf(const S3Client *client, const std::string& bucket_name, const std::string& object_name, char* buf, hsize_t length)
{
    Logger::log("------ S3Put ", object_name);
    PutObjectRequest request;
    request.SetBucket(bucket_name);
    request.SetKey(object_name);
    
    auto input_data = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream", std::stringstream::in | std::stringstream::out | std::stringstream::binary);
    input_data->write(buf, length);
    request.SetBody(input_data);

    auto outcome = client->PutObject(request);
    if (!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cerr << "ERROR: PutObject: " << object_name << " " << 
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
        return ARRAYMORPH_FAIL;
    }
    delete[] buf;
    return ARRAYMORPH_SUCCESS;
}

herr_t Operators::S3Put(const S3Client *client, const std::string& bucket_name, const std::string& object_name, Result &re)
{
    Logger::log("------ S3Put ", object_name);
    PutObjectRequest request;
    request.SetBucket(bucket_name);
    request.SetKey(object_name);
    
    auto input_data = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream", std::stringstream::in | std::stringstream::out | std::stringstream::binary);
    input_data->write(re.data.data(), re.data.size());
    request.SetBody(input_data);

    auto outcome = client->PutObject(request);
    if (!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cerr << "ERROR: PutObject: " << object_name << " " << 
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
        return ARRAYMORPH_FAIL;
    }
    return ARRAYMORPH_SUCCESS;
}

herr_t Operators::S3PutAsync(const S3Client *client, const std::string& bucket_name, const Aws::String &object_name, Result &re)
{
    Logger::log("------ S3PutAsync ", object_name);
    PutObjectRequest request;
    request.SetBucket(bucket_name);
    request.SetKey(object_name);
    
    auto input_data = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream", std::stringstream::in | std::stringstream::out | std::stringstream::binary);
    input_data->write(re.data.data(), re.data.size());
    std::shared_ptr<AsyncCallerContext> context(new AsyncWriteInput(re.data.data()));

    request.SetBody(input_data);
    client->PutObjectAsync(request, PutAsyncCallback, context);
    return ARRAYMORPH_SUCCESS;
}

// Azure

Result Operators::AzureGet(const BlobContainerClient *client, const std::string& blob_name)
{
    Result re;
    Logger::log("------ AzureGet ", blob_name);
    BlockBlobClient blclient = client->GetBlockBlobClient(blob_name);
    auto properties = blclient.GetProperties().Value;
    size_t size = properties.BlobSize;
    re.data.resize(size);
    blclient.DownloadTo(reinterpret_cast<uint8_t*>(re.data.data()), size);
    return re;
}

herr_t Operators::AzurePut(const BlobContainerClient *client, const std::string& blob_name, uint8_t *buf, size_t length)
{
    Logger::log("------ AzurePut ", blob_name);
    BlockBlobClient blclient = client->GetBlockBlobClient(blob_name);
    blclient.UploadFrom(buf, length);
    delete[] buf;
    return ARRAYMORPH_SUCCESS;
}

herr_t Operators::AzureGetAndProcess(const BlobContainerClient *client, const std::string& blob_name, const std::shared_ptr<const AsyncCallerContext> context)
{
    Logger::log("------ AzureGet ", blob_name);
    std::shared_ptr<const AsyncReadInput> input = std::static_pointer_cast<const AsyncReadInput>(context);
    BlockBlobClient blclient = client->GetBlockBlobClient(blob_name);
    auto properties = blclient.GetProperties().Value;
    size_t size = properties.BlobSize;
    uint8_t *buf = new uint8_t[size];
    blclient.DownloadTo(buf, size);
#ifdef PROCESS
    for (auto &m: input->mapping) {
        memcpy((char*)input->buf + m[1], buf + m[0], m[2]);
    }
#endif
    delete[] buf;
    return ARRAYMORPH_SUCCESS;
}

herr_t Operators::AzureGetRange(const BlobContainerClient *client, const std::string& blob_name, uint64_t beg, uint64_t end, const std::shared_ptr<const AsyncCallerContext> context) {
    Logger::log("------ AzureGetRange ", blob_name);
    BlockBlobClient blclient = client->GetBlockBlobClient(blob_name);
    
    std::shared_ptr<const AsyncReadInput> input = std::static_pointer_cast<const AsyncReadInput>(context);

    DownloadBlobToOptions options;
    options.Range = Azure::Core::Http::HttpRange();
    options.Range.Value().Offset = beg;
    size_t size = end - beg + 1;
    options.Range.Value().Length = size;

    uint8_t *buf = new uint8_t[size];
    blclient.DownloadTo(buf, size, options);
#ifdef PROCESS
    for (auto &m: input->mapping) {
        memcpy((char*)input->buf + m[1], buf + m[0], m[2]);
    }
#endif
    delete[] buf;
    return ARRAYMORPH_SUCCESS;
}

// Google Cloud

Result Operators::GCGet(gcs::Client *client, const std::string& bucket_name, const std::string& obj_name)
{
    Logger::log("------ GCGet ", obj_name);
    Result re;
    auto reader = client->ReadObject(bucket_name, obj_name);
    if (!reader) {
        std::cerr << "Failed to read object: " << reader.status().message() << std::endl;
        return re;
    }
    std::istreambuf_iterator<char> begin(reader), end;
    re.data = std::vector<char>(begin, end);

    return re;
}


herr_t Operators::GCPut(gcs::Client *client, const std::string& bucket_name, const std::string& obj_name, char* buf, hsize_t length)
{
    // cout << "bucket_name: " << bucket_name << endl;
    Logger::log("------ GCPut ", obj_name);
    auto writer = client->WriteObject(bucket_name, obj_name);
    writer.write(buf, length);
    writer.Close();
    delete[] buf;
    if (!writer.metadata()) {
        std::cerr << "Error during upload: " << writer.metadata().status() << "\n";
        return ARRAYMORPH_FAIL; // Return non-zero value to indicate failure
    }
    // auto metadata = move(writer).metadata();
    return ARRAYMORPH_SUCCESS;
}

herr_t Operators::GCGetAndProcess(gcs::Client *client, const std::string& bucket_name, const std::string& obj_name, const std::shared_ptr<const AsyncCallerContext> context)
{
    Logger::log("------ GCGet ", obj_name);
    std::shared_ptr<const AsyncReadInput> input = std::static_pointer_cast<const AsyncReadInput>(context);
    auto reader = client->ReadObject(bucket_name, obj_name);
    if (!reader) {
        std::cerr << "Failed to read object: " << reader.status().message() << std::endl;
        return ARRAYMORPH_FAIL;
    }
    std::string re_data(std::istreambuf_iterator<char>{reader}, {});
#ifdef PROCESS
    for (auto &m: input->mapping) {
        memcpy((char*)input->buf + m[1], re_data.c_str() + m[0], m[2]);
    }
#endif
    return ARRAYMORPH_SUCCESS;
}

herr_t Operators::GCGetRange(gcs::Client *client, const std::string& bucket_name, const std::string& obj_name, uint64_t beg, uint64_t end, const std::shared_ptr<const AsyncCallerContext> context) {
    Logger::log("------ GCGetRange ", obj_name);
    
    std::shared_ptr<const AsyncReadInput> input = std::static_pointer_cast<const AsyncReadInput>(context);
    auto reader = client->ReadObject(bucket_name, obj_name, gcs::ReadRange(beg, end + 1));
    if (!reader) {
        std::cerr << "Failed to read object: " << reader.status().message() << std::endl;
        return ARRAYMORPH_FAIL;
    }
    std::string re_data(std::istreambuf_iterator<char>{reader}, {});

#ifdef PROCESS
    for (auto &m: input->mapping) {
        memcpy((char*)input->buf + m[1], re_data.c_str() + m[0], m[2]);
    }
#endif
    return ARRAYMORPH_SUCCESS;
}


herr_t Operators::AzureLambda(const BlobContainerClient*client, const std::string& lambda_url, const std::string& lambda_endpoint, const std::string& query, const std::shared_ptr<const AsyncCallerContext> context){
    std::string json = "{\"query\":\"" + query + "\"}";
    Logger::log("------ LambdaURL: ", lambda_url);
    Logger::log("------ LambdaEndpoint: ", lambda_endpoint);
    Logger::log("------ query: ", json);
    const std::shared_ptr<const AsyncReadInput> input = std::static_pointer_cast<const AsyncReadInput>(context);
    try{
        httplib::SSLClient cli(lambda_url.c_str(), 443);
        cli.set_keep_alive(true);
        cli.set_read_timeout(requestTimeoutMs / 1000, 0);
        if (!cli.is_valid()) {
            throw std::invalid_argument("Client initialization failed with invalid argument");
        }
    
        auto res = cli.Post("/" + lambda_endpoint, json, "application/json");

        if (res) {
            // std::cout << "Status Code: " << res->status << std::endl;
            // std::cout << "Body: " << res->body.size() << std::endl;
#ifdef PROCESS
            for (auto &m: input->mapping) {
                memcpy((char*)input->buf + m[1], res->body.data() + m[0], m[2]);
            }
#endif
        } else {
            std::cerr << "Failed to get response. Error: " << res.error() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        Logger::log("Lambda fails, retry on GET");
        return AzureGetAndProcess(client, input->uri, input);
    }
    return ARRAYMORPH_SUCCESS;
}


herr_t Operators::GCLambda(gcs::Client *client, const std::string& lambda_url, const std::string& lambda_endpoint, const std::string& query, const std::shared_ptr<const AsyncCallerContext> context){
    std::string json = "{\"query\":\"" + query + "\"}";
    Logger::log("------ LambdaURL: ", lambda_url);
    Logger::log("------ LambdaEndpoint: ", lambda_endpoint, 443);
    Logger::log("------ query: ", json);
    const std::shared_ptr<const AsyncReadInput> input = std::static_pointer_cast<const AsyncReadInput>(context);
    try{
        httplib::SSLClient cli(lambda_url.c_str(), 443);
        cli.set_keep_alive(true);
        cli.set_read_timeout(requestTimeoutMs / 1000, 0);
        if (!cli.is_valid()) {
            throw std::invalid_argument("Client initialization failed with invalid argument");
        }
    
        auto res = cli.Post("/" + lambda_endpoint, json, "application/json");

        if (res) {
            // std::cout << "Status Code: " << res->status << std::endl;
            // std::cout << "Body: " << res->body.size() << std::endl;
#ifdef PROCESS
            for (auto &m: input->mapping) {
                memcpy((char*)input->buf + m[1], res->body.data() + m[0], m[2]);
            }
#endif
        } else {
            std::cerr << "Failed to get response. Error: " << res.error() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        Logger::log("Lambda fails, retry on GET");
        return GCGetAndProcess(client, input->bucket_name, input->uri, input);
    }
    return ARRAYMORPH_SUCCESS;
}













