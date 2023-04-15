import boto3
import sys
import os
import csv
import re
from datetime import datetime
import pytz

utc=pytz.UTC

def parse_s3_log_line(line):
	s3_line_logpats  = r'(\S+) (\S+) \[(.*?)\] (\S+) (\S+) ' \
		   r'(\S+) (\S+) (\S+) "([^"]+)" ' \
		   r'(\S+) (\S+) (\S+) (\S+) (\S+) (\S+) ' \
		   r'"([^"]+)" "([^"]+)"'
	s3_line_logpat = re.compile(s3_line_logpats)
	match = s3_line_logpat.match(line)
	if (match == None):
		print(line)
		return [None] * 17
	result = [match.group(1+n) for n in range(17)]
	return result

def pair_with_logs(rids):
	re = [["S3_LOG_BUCKET_OWNER", "S3_LOG_BUCKET", "S3_LOG_DATETIME", "S3_LOG_IP",
	"S3_LOG_REQUESTOR_ID", "S3_LOG_REQUEST_ID", "S3_LOG_OPERATION", "S3_LOG_KEY",
	"S3_LOG_HTTP_METHOD_URI_PROTO", "S3_LOG_HTTP_STATUS", "S3_LOG_S3_ERROR",
	"S3_LOG_BYTES_SENT", "S3_LOG_OBJECT_SIZE", "S3_LOG_TOTAL_TIME",
	"S3_LOG_TURN_AROUND_TIME", "S3_LOG_REFERER", "S3_LOG_USER_AGENT"]]

	if (cached):
		with open("server_logs.csv", newline='') as f:
			csv_reader = csv.reader(f, delimiter=',')
			for c in csv_reader:
				if c[0] == 'S3_LOG_BUCKET_OWNER' or c[7] == "end_to_end_2.h5/test/meta":
					continue
				re.append(c)
				rids[c[5]][-3] = datetime.strptime(c[2], "%d/%b/%Y:%H:%M:%S %z").timestamp() * 1000
				rids[c[5]][-2] = int(c[13])
				rids[c[5]][-1] = int(c[14])
			return re
	aws_key_id = os.environ['AWS_ACCESS_KEY_ID']
	aws_secret_key = os.environ['AWS_SECRET_ACCESS_KEY']
	log_bucket = os.environ['BUCKET_NAME'] + '-logs'	
	session = boto3.Session(
		aws_access_key_id=aws_key_id,
		aws_secret_access_key=aws_secret_key
	)

	s3 = boto3.client('s3')
	

	response = s3.list_objects_v2(Bucket=log_bucket)
	files = response.get("Contents")
	for file in files:
		file_name = file['Key']
		obj = s3.get_object(Bucket=log_bucket, Key=file_name)
		log_text = obj["Body"].iter_lines()
		for line in log_text:
			l = parse_s3_log_line(line.decode('utf-8'))
			if (l[5] in rids) and l[7] != "end_to_end_2.h5/test/meta":
				# info = [l[5], l[11], l[12], l[13], l[14]]
				re.append(l)
				rids[l[5]][-3] = datetime.strptime(l[2], "%d/%b/%Y:%H:%M:%S %z").timestamp() * 1000
				rids[l[5]][-2] = int(l[13])
				rids[l[5]][-1] = int(l[14])
	print(len(re), len(rids))
	# assert (len(re) - 1 == len(rids))
	return re



raw_out = sys.argv[1]
processed_out = sys.argv[2]
cached = True
rids = {}
header = ["RID", "ConnectLatency", "DnsLatency", "RequestLatency", "SslLatency", "AcquireTime", 
			"ServerReceiveTime", "ServerTotalTime", "ServerTurnaroundTime"]
with open(raw_out, 'r') as f:
	content = f.readlines()
	flag = False
	l = [None] * len(header)
	for i, c in enumerate(content):
		t = c.split()
		if c.startswith("	\"XAmzRequestId\""):
			l[0] = t[1][1:-2]
		if c.startswith("	\"ConnectLatency\""):
			l[1] = int(t[1][:-1])
		if c.startswith("	\"DnsLatency\""):
			l[2] = int(t[1][:-1])
		if  c.startswith("	\"RequestLatency\""):
			l[3] = int(t[1][:-1])
		if c.startswith("	\"AcquireConnectionLatency\""):
			l[5] = int(int(t[1][:-1]) / 1000) * 1000
		if c.startswith("	\"SslLatency\""):
			l[4] = int(t[1])
			rids[l[0]] = l
			flag = False
			l = [None] * len(header)

logs = pair_with_logs(rids)
if (not cached):
	with open("server_logs.csv", 'w') as csvf:
		csv_writer = csv.writer(csvf, delimiter=',')
		for r in logs:
			csv_writer.writerow(r)

out = [header]
for r in rids:
	if rids[r][-1] != None:
		out.append(rids[r])
print(len(out))

min_time = min([t[5] for t  in out[1:]])


for o in out[1:]:
	o[-3] = int(o[-3] - min_time)
	o[5] = int(o[5] - min_time)
	assert(o[-3] >= 0)
	assert(o[-3] >= o[5])


with open(processed_out, 'w') as csvf:
	csv_writer = csv.writer(csvf, delimiter=',')
	for r in out:
		csv_writer.writerow(r)








	










