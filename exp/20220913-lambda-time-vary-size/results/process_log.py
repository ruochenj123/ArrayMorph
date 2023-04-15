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

def get_logs():
	re = [["S3_LOG_BUCKET_OWNER", "S3_LOG_BUCKET", "S3_LOG_DATETIME", "S3_LOG_IP",
	"S3_LOG_REQUESTOR_ID", "S3_LOG_REQUEST_ID", "S3_LOG_OPERATION", "S3_LOG_KEY",
	"S3_LOG_HTTP_METHOD_URI_PROTO", "S3_LOG_HTTP_STATUS", "S3_LOG_S3_ERROR",
	"S3_LOG_BYTES_SENT", "S3_LOG_OBJECT_SIZE", "S3_LOG_TOTAL_TIME",
	"S3_LOG_TURN_AROUND_TIME", "S3_LOG_REFERER", "S3_LOG_USER_AGENT"]]

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
			re.append(l)
	return re

logs = get_logs()
n = 0
filtered_logs = [logs[0]]
for l in logs:
	if (l[7].startswith("chunk_size_16777216.h5/test/")):
		filtered_logs.append(l)
# print(n)

with open("logs.csv", 'w') as csvf:
	csv_writer = csv.writer(csvf, delimiter=',')
	for r in filtered_logs:
		csv_writer.writerow(r)








	










