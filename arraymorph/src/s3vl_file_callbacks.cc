#include "s3vl_file_callbacks.h"
#include <stdlib.h>
#include "logger.h"
#include "constants.h"
#include "operators.h"

CloudClient get_client() {
	Logger::log("Init cloud clients");
	// AWS connection
	CloudClient client;
	if (SP == SPlan::S3) {
		std::string access_key = getenv("AWS_ACCESS_KEY_ID");
		std::string secret_key = getenv("AWS_SECRET_ACCESS_KEY");
		Aws::Auth::AWSCredentials cred(access_key, secret_key);
		std::unique_ptr<Aws::Client::ClientConfiguration> s3ClientConfig = std::make_unique<Aws::Client::ClientConfiguration>();
		s3ClientConfig->scheme = Scheme::HTTPS;
		s3ClientConfig->maxConnections = s3Connections;
		s3ClientConfig->requestTimeoutMs = requestTimeoutMs;
		s3ClientConfig->connectTimeoutMs = connectTimeoutMs;
		s3ClientConfig->retryStrategy = std::make_shared<Aws::Client::StandardRetryStrategy>(retries);
#ifdef POOLEXECUTOR
		s3ClientConfig->executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("test", poolSize);
#endif
		Logger::log("------ Create Client config: maxConnections=", s3ClientConfig->maxConnections);
    	client = std::make_unique<Aws::S3::S3Client>(cred, std::move(*s3ClientConfig), Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, true);
		s3ClientConfig.reset();
	}
    // GCS connection
	else if (SP == SPlan::GOOGLE) {
		std::string gc_connection_file = getenv("GOOGLE_CLOUD_STORAGE_JSON");
		auto is = std::ifstream(gc_connection_file);
		auto json_string =
				std::string(std::istreambuf_iterator<char>(is.rdbuf()), {});
		auto credentials =
				google::cloud::MakeServiceAccountCredentials(json_string);
		auto retry_policy = google::cloud::storage::LimitedErrorCountRetryPolicy(retries).clone();
		auto backoff_policy = google::cloud::storage::ExponentialBackoffPolicy(
			std::chrono::milliseconds(200),  
			std::chrono::seconds(2),         
			2.0                              
		).clone();
		client = std::make_unique< gcs::Client>(
				google::cloud::Options{}
				.set<google::cloud::UnifiedCredentialsOption>(credentials)
				.set<gcs::ConnectionPoolSizeOption>(gcConnections)
				.set<gcs::RetryPolicyOption>(std::move(retry_policy))
            	.set<gcs::BackoffPolicyOption>(std::move(backoff_policy)));
	}
	// Azure connection
	else {
		std::string azure_connection_string = getenv("AZURE_STORAGE_CONNECTION_STRING");

		Azure::Core::Http::Policies::RetryOptions retryOptions;
		retryOptions.MaxRetries = retries;
		retryOptions.RetryDelay = std::chrono::milliseconds(200); 
		retryOptions.MaxRetryDelay = std::chrono::seconds(2); 

		Azure::Storage::Blobs::BlobClientOptions clientOptions;
		clientOptions.Retry = retryOptions;
		client = std::make_unique<BlobContainerClient>(BlobContainerClient::CreateFromConnectionString(azure_connection_string, BUCKET_NAME, clientOptions));
	}
	return client;
}

void* S3VLFileCallbacks::S3VL_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req) {
	S3VLFileObj *ret_obj = new S3VLFileObj();
	std::string path(name);
	if (path.rfind("./", 0) == 0) {
		path = path.substr(2);
	}
	ret_obj->name = path;
	Logger::log("------ Create File:", path);
	if (std::holds_alternative<std::monostate>(global_cloud_client)) {
		global_cloud_client = get_client();
	}
	return (void*)ret_obj;
}
void* S3VLFileCallbacks::S3VL_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req) {
	S3VLFileObj *ret_obj = new S3VLFileObj();
	std::string path(name);
	Logger::log("------ Open File:", path);
	if (path.rfind("./", 0) == 0) {
		path = path.substr(2);
	}
	ret_obj->name = path;
	if (std::holds_alternative<std::monostate>(global_cloud_client)) {
		global_cloud_client = get_client();
	}
	return (void*)ret_obj;

}
herr_t S3VLFileCallbacks::S3VL_file_close(void *file, hid_t dxpl_id, void **req) {
	S3VLFileObj *file_obj = (S3VLFileObj*)file;
	Logger::log("------ Close File: ", file_obj->name);
	delete file_obj;
	return ARRAYMORPH_SUCCESS;
}

herr_t S3VLFileCallbacks::S3VL_file_get(void *file, H5VL_file_get_args_t *args, hid_t dxpl_id, void **req) {
	Logger::log("------ Get File");
	if (args->op_type == H5VL_file_get_t::H5VL_FILE_GET_FCPL) {
		hid_t fcpl_id = H5Pcreate(H5P_FILE_CREATE);
		args->args.get_fcpl.fcpl_id = H5Pcopy(fcpl_id);
		// args->args.get_fcpl.fcpl_id = 1;
	}
	return ARRAYMORPH_SUCCESS;
}
