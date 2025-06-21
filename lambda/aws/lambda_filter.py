import boto3
import json
import struct
from botocore.config import Config
import time
def reduce(a, b):
	re = []
	for i in a:
		for j in b:
			re.append(i + j)
	return re

def get_offsets(meta):
	ndims = meta["ndims"]
	shape = meta["shape"]
	ranges = meta["ranges"]
	assert(len(shape) == ndims)
	assert(len(ranges) == ndims)

	offsets_per_dim = [1] * ndims
	offsets = []
	for i in range(ndims - 2, -1, -1):
		offsets_per_dim[i] = offsets_per_dim[i + 1] * shape[i + 1]
	for i in range(ndims - 1):
		tmp = []
		for j in range(ranges[i][0], ranges[i][1] + 1):
			tmp.append(j * offsets_per_dim[i])
		offsets.append(tmp)
	re = [0]
	for i in range(ndims - 1):
		re = reduce(re, offsets[i])
	for i in range(len(re)):
		re[i] += ranges[ndims - 1][0]

	return re

def hyperslab_read(data, meta):
# 	print("meta: ", meta)
	data_size = meta["data_size"]
	ranges = meta["ranges"]
	ndims = meta["ndims"]
	interval = ranges[ndims - 1][1] - ranges[ndims - 1][0] + 1
	offsets = get_offsets(meta)
# 	print("interval: ", interval)
# 	print("offsets: ", offsets)
	return_size = len(offsets) * interval * data_size
# 	print("return_size：", return_size)
	re = bytearray(b"0" * return_size)
	for i, o in enumerate(offsets):
		re[(i * interval * data_size) : (i + 1) * interval * data_size] = data[(o * data_size) : ((o + interval) * data_size)]
	return bytes(re)


def lambda_handler(event, context):
	start = time.time()
	print('event', event)
	object_get_context = event["getObjectContext"]
	request_route = object_get_context["outputRoute"]
	request_token = object_get_context["outputToken"]
	s3_url = object_get_context["inputS3Url"]
	
	# get object name and query
	file_name = event["userRequest"]["url"].split('.com/')[1]
	# qfile_name = file_name + ".query"
	
	obj_prefix, query = file_name.rsplit("/", 1)
	m = query.split("-")
	obj_name = obj_prefix + '/' + m[0]

	data_size = int(m[1])
	ndims = int(m[2])
	q = {"ndims": ndims, "data_size": data_size}
	shape = []
	ranges = []
	for i in range(ndims):
		shape.append(int(m[i + 3]))
		ranges.append([int(m[i * 2 + 3 + ndims]), int(m[i * 2 + 4 + ndims])])
	q["shape"] = shape
	q["ranges"] = ranges
	
	s3 = boto3.client('s3', 
	config=Config(connect_timeout=300, read_timeout=300, retries={'max_attempts': 1}))
	
	
	
	response = s3.get_object(Bucket="sci4dda-vol-on-ood", Key=obj_name)
	data = bytearray(response['Body'].read())
	get_data_time = time.time()
	print("got_data_time: ", get_data_time - start)
	processed = hyperslab_read(data, q)
	processed_time = time.time()
	print("processing time:", processed_time - get_data_time)
	# time.sleep(2)
	s3.write_get_object_response(
		Body=processed,
		RequestRoute=request_route,
		RequestToken=request_token)
	
	write_back_time = time.time()
	print("write_back_time", write_back_time - processed_time)
	return {'status_code': 200}