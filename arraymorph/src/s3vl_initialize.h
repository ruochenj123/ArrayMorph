#ifndef S3VL_INITIALIZE
#define S3VL_INITIALIZE

#include <aws/core/Aws.h>
#include <hdf5.h>
#include "constants.h"
#include "s3vl_vol_connector.h"
#include <cstdlib>
class S3VLINITIALIZE
{
public:
    static herr_t s3VL_initialize_init(hid_t vipl_id);
    static herr_t s3VL_initialize_close();
};

std::optional<std::string> getEnv(const char* var) {
    const char* val = std::getenv(var);
    return val ? std::optional<std::string>(val) : std::nullopt;
}

inline herr_t S3VLINITIALIZE::s3VL_initialize_init(hid_t vipl_id) {
    Aws::SDKOptions options;
    options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Off;
    Aws::InitAPI(options);
    Logger::log("------ Init VOL");
    std::optional<std::string> platform = getEnv("STORAGE_PLATFORM");
    if (platform.has_value()) {
        std::string platform_str = platform.value();
        if (platform_str == "S3")
            Logger::log("------ Using S3");
        else if (platform_str == "Azure") {
            Logger::log("------ Using Azure");
            SP = SPlan::AZURE_BLOB;
        }
        else if (platform_str == "Google") {
            Logger::log("------ Using GCS");
            SP = SPlan::GOOGLE;
        }
        else {
            Logger::log("------ Unsupported platform");
            return ARRAYMORPH_FAIL;
        }
    }
    else {
        Logger::log("------ Using default platform S3");
    }
    std::optional<std::string> bucket_name = getEnv("BUCKET_NAME");
    if (bucket_name.has_value()) {
        BUCKET_NAME = bucket_name.value();
        Logger::log("------ Using bucket", BUCKET_NAME);
    }
    else {
        Logger::log("------ Bucekt not set");
        return ARRAYMORPH_FAIL;
    }
    std::optional<std::string> single_plan = getEnv("SINGLE_PLAN");
    if (single_plan.has_value()) {
        std::string single_plan_str = single_plan.value();
        if (single_plan_str == "GET") {
            Logger::log("------ Using ReadAll");
            SINGLE_PLAN = QPlan::GET;
        }
        else if (single_plan_str == "MERGE") {
            Logger::log("------ Using MERGE");
            SINGLE_PLAN = QPlan::MERGE;
        }
        else if (single_plan_str == "LAMBDA") {
            Logger::log("------ Using LAMBDA");
            SINGLE_PLAN = QPlan::LAMBDA;
        }
        else if (single_plan_str == "MULTI_FETCH") {
            Logger::log("------ Using Multi-fetch");
            SINGLE_PLAN = QPlan::MULTI_FETCH;
        }
	else if (single_plan_str == "ARRAYMORPH")
		Logger::log("------ Using ArrayMorph");
        else {
            Logger::log("------ Unsupported method");
            return ARRAYMORPH_FAIL;
        }
    }
    else {
         Logger::log("------ Using arrayMorph");
    }
    return S3_VOL_CONNECTOR_VALUE;
}

inline herr_t S3VLINITIALIZE::s3VL_initialize_close() {
    Logger::log("------ Close VOL");
    // Aws::ShutdownAPI(opt);
    return ARRAYMORPH_SUCCESS;
}




#endif
