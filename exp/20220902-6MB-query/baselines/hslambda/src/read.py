import boto3
import json
import time

s = time.time()

lambda_client = boto3.client('lambda')
test_event = {
  "method": "GET",
  "path": "/datasets/d-2f22962d-e0592840-e15c-625d94-9e4cbf/value",
  "headers": {
    "accept": "application/octet-stream"
  },
  "params": {
    "domain": "/home/admin/hsds.hdf5",
    "select": "[0:32767, 0:47]",
    "bucket": "hsds-hyperslab-test"
  }
}

re = lambda_client.invoke(
        FunctionName='hslambda',
        Payload=json.dumps(test_event)
        )
e = time.time()
print("total time: ", e - s)
