import azure.functions as func
import logging
from azure.storage.blob import BlobServiceClient, BlobClient, ContainerClient
import os
import time

app = func.FunctionApp(http_auth_level=func.AuthLevel.FUNCTION)

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
	# print("meta: ", meta)
	data_size = meta["data_size"]
	ranges = meta["ranges"]
	ndims = meta["ndims"]
	interval = ranges[ndims - 1][1] - ranges[ndims - 1][0] + 1
	offsets = get_offsets(meta)
	# print("interval: ", interval)
	# print("offsets: ", offsets)
	return_size = len(offsets) * interval * data_size
	# print("return_size：", return_size)
	re = bytearray(b"0" * return_size)
	for i, o in enumerate(offsets):
		re[(i * interval * data_size) : (i + 1) * interval * data_size] = data[(o * data_size) : ((o + interval) * data_size)]
	return bytes(re)


@app.route(route="test")
def test(req: func.HttpRequest) -> func.HttpResponse:
	logging.info('Python HTTP trigger function processed a request.')
	start = time.time()
	name = req.params.get('name')
	req_body = req.get_json()
	# print("req_body:", req_body)
	
	q = req_body["query"]
	# qfile_name = file_name + ".query"
	
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

	blob_service_client = BlobServiceClient.from_connection_string(os.environ['AZURECONNECTIONSTRING'])
	container_client = blob_service_client.get_container_client(os.environ['CONTAINERNAME'])
	blob_client = container_client.get_blob_client(obj_name)
	streamdownloader = blob_client.download_blob()
	data = streamdownloader.readall()
	after_download = time.time()
	processed = hyperslab_read(data, q)
	after_process = time.time()
	# print("processed: ", processed)
	# print("processed size:", len(processed))
	logging.info("process meta time: %f", process_meta - start)
	logging.info("download time: %f", after_download - process_meta)
	logging.info("process time: %f", after_process - after_download)
	return func.HttpResponse(
		body=processed,
		status_code=200
	)