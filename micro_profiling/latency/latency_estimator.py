import os
os.environ["SINGLE_PLAN"]="LAMBDA"
import h5py
import numpy as np
import math
import time
import random
from datetime import datetime, timezone
import boto3
from google.cloud import storage
from azure.storage.blob import BlobServiceClient
from s3logparse import s3logparse
import argparse
import json
from sklearn.linear_model import LinearRegression

import query_log
from query_log import S3LogQuery


parser = argparse.ArgumentParser()
parser.add_argument("--platform", choices=["S3", "Google", "Azure"])
parser.add_argument("--band", type=float)
parser.add_argument
args = parser.parse_args()


# used for parsing s3 server-side log, accurate but has high latency (need 1-2 hours to wait for server-side log generation)
def process_s3_logs(log_bucket_name, key):
    res = []
    try:
        aws_access_key_id = os.getenv('AWS_ACCESS_KEY_ID')
        aws_secret_access_key = os.getenv('AWS_SECRET_ACCESS_KEY')

        s3 = boto3.client(
            's3',
            aws_access_key_id=aws_access_key_id,
            aws_secret_access_key=aws_secret_access_key
        )
        # List objects (log files) in the S3 bucket
        logs = s3.list_objects_v2(Bucket=log_bucket_name)

        if 'Contents' not in logs:
            print(f"No log files found in bucket '{log_bucket_name}'.")
            return

        # Iterate through each log file
        for log_obj in logs['Contents']:
            log_key = log_obj['Key']
            response = s3.get_object(Bucket=log_bucket_name, Key=log_key)
            log_lines = response['Body'].read().decode('utf-8').splitlines()
            for log_entry in s3logparse.parse_log_lines(log_lines):
                if log_entry.s3_key != None and log_entry.s3_key.startswith(key) and log_entry.turn_around_time != 0:
                    res.append(log_entry.turn_around_time)
        return res

    except ClientError as e:
        print(f"Error occurred: {e}")

# estimating the latency by reducing data transfer time, less accurate but faster
def fetch_and_estimate_latency(cloud_platform, bucket_name, chunk_size, estimated_band):
    # fetch a single chunk
    blob_name = f"micro/microbenchmark_single_chunk.h5/{chunk_size}/0"
    start_time = time.time() 

    if cloud_platform.lower() == "s3":
        aws_access_key_id = os.getenv('AWS_ACCESS_KEY_ID')
        aws_secret_access_key = os.getenv('AWS_SECRET_ACCESS_KEY')

        s3_client = boto3.client(
            's3',
            aws_access_key_id=aws_access_key_id,
            aws_secret_access_key=aws_secret_access_key
        )
        response = s3_client.get_object(Bucket=bucket_name, Key=blob_name)
        blob_data = response["Body"].read()

    elif cloud_platform.lower() == "google":
        client = storage.Client.from_service_account_json(os.getenv("GOOGLE_CLOUD_STORAGE_JSON"))
        bucket = client.bucket(bucket_name)
        blob = bucket.blob(blob_name)
        blob_data = blob.download_as_bytes()

    elif cloud_platform.lower() == "azure":
        connection_string = os.getenv("AZURE_CONNECTION_STRING")
        blob_service_client = BlobServiceClient.from_connection_string(connection_string)
        blob_client = blob_service_client.get_blob_client(container=bucket_name, blob=blob_name)
        blob_data = blob_client.download_blob().readall()

    else:
        raise ValueError("Unsupported cloud platform. Choose from 's3', 'google', or 'azure'.")

    end_time = time.time()
    latency_ms = (end_time - start_time) * 1000 - chunk_size / estimated_band * 1000

    return latency_ms



def execute_lambdas(uri, chunk_size, n):
    runs = []
    f = h5py.File(uri + "/microbenchmark_single_chunk.h5", "r")
    dset = f[str(chunk_size)]
    shape = int(math.sqrt(chunk_size * 1024 * 1024 / 4))
    for i in range(n):
        start_stamp = datetime.now(timezone.utc)
        start_millis = int(start_stamp.timestamp() * 1000)
        x = random.randint(0, shape / 2)
        y = random.randint(0, shape / 2)
        data = dset[0:x, 0:y]
        end_stamp = datetime.now(timezone.utc)
        end_millis = int(start_stamp.timestamp() * 1000)
        runs.append((start_millis, end_millis))
        # sleep 10s for querying logging between runs
        time.sleep(2)
    return runs

s3_q = """
	fields @timestamp, @billedDuration, @message
	|fields toMillis(@timestamp) as millis
"""
s3logquery = S3LogQuery()

def profile_lambdas(stamp, platform):
    if platform.lower() == "s3":
        res = s3logquery.run(stamp[0], stamp[1], s3_q)
        for log in res["results"]:
            for field in log:
                if field["field"] == "@billedDuration":
                    return float(field["value"])
    elif platform.lower() == "google":
        return query_log.google_log_query(stamp[0], stamp[1])
    elif platform.lower() == "azure":
        return query_log.azure_log_query(stamp[0], stamp[1])
    else:
        raise ValueError("Unsupported cloud platform. Choose from 's3', 'google', or 'azure'.")




if __name__ == "__main__":
    bucket_name = os.getenv("BUCKET_NAME")
    n = 10
    chunk_sizes = [2, 4, 8, 16, 32, 64]
    res = {}

    # get latency
    profile_latency_start = time.time()
    for size in chunk_sizes:
        for i in range(n):
            res.setdefault(size, []).append(fetch_and_estimate_latency(args.platform, bucket_name, size, args.band))
        print(f"Average latency for chunk size {size} MB: {sum(res[size]) / n} ms")
    # print(f"Raw output: {res}")
    

    # lambda latency
    stamps = {}
    res = {}
    for chunk_size in chunk_sizes:
        stamps[chunk_size] = execute_lambdas("micro", chunk_size, n)
    time.sleep(10)

    print("Lambda execution completed.")

    # profiling
    for k, v in stamps.items():
        for s in v:
            res.setdefault(k, []).append(profile_lambdas(s, args.platform))
    print(res)
    # linear regression
    X = []
    y = []
    for size, times in res.items():
        X.extend([size] * len(times))
        y.extend(times)
    
    X = np.array(X).reshape(-1, 1)
    y = np.array(y, dtype=float)
    mask = ~np.isnan(y)
    X, y = X[mask], y[mask]
    model = LinearRegression()
    model.fit(X, y)

    a = model.coef_[0]
    b = model.intercept_

    profile_latency_end = time.time()
    print(f"Estimated equation: t = {a:.4f} * size + {b:.4f}")
    print(f"Estimating GET latency completed. Using {profile_latency_end - profile_latency_start} seconds.")