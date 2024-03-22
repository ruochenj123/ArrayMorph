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
    static Aws::SDKOptions opt;
    static herr_t s3VL_initialize_init(hid_t vipl_id);
    static herr_t s3VL_initialize_close();
};

Aws::SDKOptions S3VLINITIALIZE::opt = Aws::SDKOptions();
inline herr_t S3VLINITIALIZE::s3VL_initialize_init(hid_t vipl_id) {
    Aws::InitAPI(opt);
    Logger::log("------ Init VOL");
    const char* platform = std::getenv("STORAGE_PLATFORM");
    if (platform == NULL || platform == "S3") {
        Logger::log("------ Using S3");
        // SP = SPlan::S3;
    }
    else if (platform == "Azure") {
        Logger::log("------ Using Azure");
        SP = SPlan::AZURE_BLOB;
    }
    else if (platform == "Google") {
        Logger::log("------ Using GCS");
        SP = SPlan::GOOGLE;
    }
    else {
        Logger::log("------ Unsupported platform");
        return FAIL;
    }
    const char* single_plan = std::getenv("SINGLE_PLAN");
    if (single_plan == NULL) {
        Logger::log("------ Using arrayMorph");
    }
    else if (single_plan == "GET") {
        Logger::log("------ Using ReadAll");
        SINGLE_PLAN = QPlan::GET;
    }
    else if (single_plan == "MERGE") {
        Logger::log("------ Using MERGE");
        SINGLE_PLAN = QPlan::MERGE;
    }
    else if (single_plan == "LAMBDA") {
        Logger::log("------ Using LAMBDA");
        SINGLE_PLAN = QPlan::LAMBDA;
    }
    else {
        Logger::log("------ Unsupported method");
        return FAIL;
    }
    return S3_VOL_CONNECTOR_VALUE;
}

inline herr_t S3VLINITIALIZE::s3VL_initialize_close() {
    Logger::log("------ Close VOL");
    // Aws::ShutdownAPI(opt);
    return SUCCESS;
}




#endif