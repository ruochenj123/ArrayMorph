#include "operators.h"
#include "logger.h"
#include <assert.h>
#include <time.h>
#include <chrono>

mutex output_mutex;
Profiler S3profiler;
void PutAsyncCallback(const Aws::S3::S3Client* s3Client, 
    const Aws::S3::Model::PutObjectRequest& request, 
    const Aws::S3::Model::PutObjectOutcome& outcome,
    const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) {
    if (outcome.IsSuccess()) {
        Logger::log("write async successfully: ", request.GetKey());
        finish++;
    }
    else {
        Logger::log("write async failed: ", request.GetKey());
    }
    const std::shared_ptr<const AsyncWriteInput> input = static_pointer_cast<const AsyncWriteInput>(context);
    delete[] input->buf;
}


void Operators::GetAsyncCallback(const Aws::S3::S3Client* s3Client, 
    const Aws::S3::Model::GetObjectRequest& request, 
    Aws::S3::Model::GetObjectOutcome outcome,
    const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context) {
    const std::shared_ptr<const AsyncReadInput> input = static_pointer_cast<const AsyncReadInput>(context);
    if (outcome.IsSuccess()) {
        auto& file = outcome.GetResultWithOwnership().GetBody();
        file.seekg(0, file.end);
        size_t length = file.tellg();
        file.seekg(0, file.beg);
      
        // output_mutex.lock();
        // cout << "IO size: " << length << " bytes" << endl;
        // output_mutex.unlock();
#ifdef PROCESS
        if (length < 1024 * 1024 * 1024) {
            char* buf = new char[length];
            file.read(buf, length);
            for (auto &m: input->mapping) {
                memcpy((char*)input->buf + m[1], buf + m[0], m[2]);
            }
            delete[] buf;
        }
        else {
            for (auto &m: input->mapping) {
                file.seekg(m[0], file.beg);
                file.read((char*)input->buf + m[1], m[2]);
            }
        }
#endif
    } else {
        auto err = outcome.GetError();
        cout << request.GetKey() << endl;
        std::cout << "Error: GetObject: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    }
    finish++;
    Logger::log("read async successfully: ", request.GetKey());

}


herr_t Operators::S3GetAsync(S3Client *client, string bucket_name, const Aws::String &object_name,
                    std::shared_ptr<AsyncCallerContext> input)
{

    Logger::log("------ S3getAsync ", object_name);
    GetObjectRequest request;
    request.SetBucket(bucket_name);
    request.SetKey(object_name);
    auto handler = [&](const HttpRequest* http_req,
                        HttpResponse* http_resp,
                        long long) {
        if (http_resp->HasHeader("triggered"))
            return;
        http_resp->AddHeader("triggered", "Yes");
        auto headers = http_resp->GetHeaders();
        cout << "rid: " << headers["x-amz-request-id"] << endl;
    };
    // request.SetDataReceivedEventHandler(std::move(handler));
    client->GetObjectAsync(request, GetAsyncCallback, input);
    return SUCCESS;
}

Result Operators::S3Get(S3Client *client, string bucket_name, const Aws::String &object_name)
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
        char* buf = new char[length];
        file.read(buf, length);
        re.length = length;
        re.data = buf;
    } else {
        auto err = outcome.GetError();
        std::cout << "Error: GetObject: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    }
    return re;
}

herr_t Operators::S3Delete(const S3Client *client, string bucket_name, const Aws::String &object_name) {
    Logger::log("------ S3Delete ", object_name);
    DeleteObjectRequest request;
    request.SetBucket(bucket_name);
    request.SetKey(object_name);

    auto outcome = client->DeleteObject(request);
    if (!outcome.IsSuccess())
    {
        auto err = outcome.GetError();
        std::cout << "Error: DeleteObject: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
        return FAIL;
    }
    return SUCCESS;
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
//         return FAIL;
//     }
//     return SUCCESS;
// }

herr_t Operators::S3PutBuf(S3Client *client, string bucket_name, string object_name, char* buf, hsize_t length)
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
        std::cout << "ERROR: PutObject: " << 
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
        return FAIL;
    }
    delete[] buf;
    return SUCCESS;
}

herr_t Operators::S3Put(S3Client *client, string bucket_name, string object_name, Result &re)
{
    Logger::log("------ S3Put ", object_name);
    PutObjectRequest request;
    request.SetBucket(bucket_name);
    request.SetKey(object_name);
    
    auto input_data = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream", std::stringstream::in | std::stringstream::out | std::stringstream::binary);
    input_data->write(re.data, re.length);
    request.SetBody(input_data);

    auto outcome = client->PutObject(request);
    if (!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cout << "ERROR: PutObject: " << 
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
        return FAIL;
    }
    delete[] re.data;
    return SUCCESS;
}

herr_t Operators::S3PutAsync(S3Client *client, string bucket_name, const Aws::String &object_name, Result &re)
{
    Logger::log("------ S3PutAsync ", object_name);
    PutObjectRequest request;
    request.SetBucket(bucket_name);
    request.SetKey(object_name);
    
    auto input_data = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream", std::stringstream::in | std::stringstream::out | std::stringstream::binary);
    input_data->write(re.data, re.length);
    shared_ptr<AsyncCallerContext> context(new AsyncWriteInput(re.data));

    request.SetBody(input_data);
    client->PutObjectAsync(request, PutAsyncCallback, context);
    return SUCCESS;
}

// Azure
herr_t Operators::AzurePut(BlobContainerClient &client, string blob_name, uint8_t *buf, size_t length)
{
    Logger::log("------ AzurePut ", blob_name);
    BlockBlobClient blclient = client.GetBlockBlobClient(blob_name);
    blclient.UploadFrom(buf, length);
    delete[] buf;
    return SUCCESS;
}

herr_t Operators::AzureGet(BlobContainerClient &client, string blob_name, shared_ptr<AsyncCallerContext> context)
{
    Logger::log("------ AzureGet ", blob_name);
    std::shared_ptr<const AsyncReadInput> input = static_pointer_cast<const AsyncReadInput>(context);
    BlockBlobClient blclient = client.GetBlockBlobClient(blob_name);
    auto properties = blclient.GetProperties().Value;
    int size = properties.BlobSize;
    uint8_t *buf = new uint8_t[size];
    blclient.DownloadTo(buf, size);
#ifdef PROCESS
    for (auto &m: input->mapping) {
        memcpy((char*)input->buf + m[1], buf + m[0], m[2]);
    }
#endif
    delete[] buf;
    return SUCCESS;
}

herr_t Operators::AzureGetRange(BlobContainerClient &client, string blob_name, uint64_t beg, uint64_t end, shared_ptr<AsyncCallerContext> context) {
    Logger::log("------ AzureGetRange ", blob_name);
    BlockBlobClient blclient = client.GetBlockBlobClient(blob_name);
    
    std::shared_ptr<const AsyncReadInput> input = static_pointer_cast<const AsyncReadInput>(context);

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
    return SUCCESS;
}

// Google Cloud
herr_t Operators::GCPut(gcs::Client *client, string bucket_name, string obj_name, char* buf, hsize_t length)
{

    Logger::log("------ GCPut ", obj_name);
    auto writer = client->WriteObject(bucket_name, obj_name);
    writer.write(buf, length);
    writer.Close();
    delete[] buf;
    return SUCCESS;
}

herr_t Operators::GCGet(gcs::Client *client, string bucket_name, string obj_name, shared_ptr<AsyncCallerContext> context)
{
    Logger::log("------ GCGet ", obj_name);
    std::shared_ptr<const AsyncReadInput> input = static_pointer_cast<const AsyncReadInput>(context);
    auto reader = client->ReadObject(bucket_name, obj_name);
    if (!reader)
        return FAIL;
    string re_data(std::istreambuf_iterator<char>{reader}, {});
#ifdef PROCESS
    for (auto &m: input->mapping) {
        memcpy((char*)input->buf + m[1], re_data.c_str() + m[0], m[2]);
    }
#endif
    return SUCCESS;
}

herr_t Operators::GCGetRange(gcs::Client *client, string bucket_name, string obj_name, uint64_t beg, uint64_t end, shared_ptr<AsyncCallerContext> context) {
    Logger::log("------ GCGetRange ", obj_name);
    
    std::shared_ptr<const AsyncReadInput> input = static_pointer_cast<const AsyncReadInput>(context);
    auto reader = client->ReadObject(bucket_name, obj_name, gcs::ReadRange(beg, end + 1));
    if (!reader)
        return FAIL;
    string re_data(std::istreambuf_iterator<char>{reader}, {});

#ifdef PROCESS
    for (auto &m: input->mapping) {
        memcpy((char*)input->buf + m[1], re_data.c_str() + m[0], m[2]);
    }
#endif
    return SUCCESS;
}

static size_t GCLambdaWriteback(void* data, size_t size, size_t nmemb, shared_ptr<const AsyncReadInput> input) {
    size_t realsize = size * nmemb;
    // shared_ptr<const AsyncReadInput> input = static_pointer_cast<const AsyncReadInput>(userp);
#ifdef PROCESS
    for (auto &m: input->mapping) {
        memcpy((char*)input->buf + m[1], (char*)data + m[0], m[2]);
    }
#endif
    return realsize;
}

static size_t
mem_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct response *mem = (struct response *)userp;

    char *ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        Logger::log("not enough memory (realloc returned NULL)");
        return 0;
    }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

herr_t Operators::GCLambda(string lambda_url, string query, shared_ptr<AsyncCallerContext> context){
    string json = "{\"query\":\"" + query + "\"}";
    Logger::log("------ GCLambda: ", lambda_url);
    Logger::log("------ query: ", json);
    std::shared_ptr<const AsyncReadInput> input = static_pointer_cast<const AsyncReadInput>(context);
    struct response chunk = {.memory = (char*)malloc(0), .size = 0};

    CURL* curl = curl_easy_init();

    if (!curl) {
        Logger::log("init curl failed");
        return FAIL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, lambda_url.c_str());

    struct curl_slist* headers = NULL;

    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mem_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    CURLcode ret = curl_easy_perform(curl);
    if (ret != CURLE_OK) {
        Logger::log("return curl failed");
        return FAIL;
    }
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

#ifdef PROCESS
    for (auto &m: input->mapping) {
        memcpy((char*)input->buf + m[1], chunk.memory + m[0], m[2]);
    }
#endif
    free(chunk.memory);

    return SUCCESS;
}













