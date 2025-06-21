import functions_framework
from flask import make_response
import logging
import os
import time
from google.cloud import storage
logging.basicConfig(level=logging.INFO)

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

@functions_framework.http
def hello_http(request):
	"""HTTP Cloud Function.
	Args:
		request (flask.Request): The request object.
		<https://flask.palletsprojects.com/en/1.1.x/api/#incoming-request-data>
	Returns:
		The response text, or any set of values that can be turned into a
		Response object using `make_response`
		<https://flask.palletsprojects.com/en/1.1.x/api/#flask.make_response>.
	"""
	start = time.time()
	request_json = request.get_json(silent=True)
	request_args = request.args
	# print(request_json)
	# print(request_args)
	q = request_json["query"]
	# q = request_args["query"]
	# qfile_name = file_name + ".query"
	print(q)
	obj_prefix, query = q.rsplit("/", 1)
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
	process_meta = time.time()

	client = storage.Client()
	# bucket = client.bucket(os.environ['CONTAINERNAME'])
	bucket = client.bucket("sci4dda-vol-on-ood")
	blob = bucket.blob(obj_name)
	data = blob.download_as_bytes()

	after_download = time.time()
	processed = hyperslab_read(data, q)
	after_process = time.time()
	# print("processed: ", processed)
	# print("processed size:", len(processed))
	print("process meta time: %f", process_meta - start)
	print("download time: %f", after_download - process_meta)
	print("process time: %f", after_process - after_download)
	print("length: %f", len(processed))
	print(type(processed))
	response = make_response(processed)
	response.status_code = 200
	response.headers['Content-Type'] = 'application/octet-stream'
	
	return response
