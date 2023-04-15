import boto3
from datetime import datetime, timedelta
import time
import os

class LogQuery:
	def __init__(self):
		self.client = boto3.client('logs',
			region_name = "us-east-1",
		    aws_access_key_id=os.getenv("AWS_ACCESS_KEY_ID"),
		    aws_secret_access_key=os.getenv("AWS_SECRET_ACCESS_KEY"),
		)
		self.log_group = "/aws/lambda/test-fn"
	def run(self, start, end, query):
		start_query_response = self.client.start_query(
		    logGroupName=self.log_group,
		    startTime=int(start) // 1000 - 1,
		    endTime=int(end) // 1000 + 1,
		    queryString=query,
		)
		q_id = start_query_response["queryId"]
		response = None
		while response == None or response["status"] == "Running":
			time.sleep(1)
			response = self.client.get_query_results(
					queryId=q_id
				)
		return response


if __name__ == '__main__':
	query = """
		fields @timestamp, @billedDuration, @memorySize, @maxMemoryUsed, @message
		| filter(@billedDuration > 0)
		| avg(@billedDuration)
	"""
	start = "1663081572516"
	end = "1663081604132"
	logquery = LogQuery()
	res = logquery.run(start, end, query)
	flag = True
	print(res["results"][0][0]["value"])
	# for log in res["results"]:
	# 	for field in log:
	# 		if field["field"] == "@message" and "Init Duration" in field["value"]:
	# 			flag = False
	# 			print("cold start")
	# 			break
	# 		if field["field"] == "@billedDuration":
	# 			print("billedDuration: ", field["value"])
	# 		elif field["field"] == "@message" and field["value"].startswith("write_back_time"):
	# 			print("write_back_time: ", field["value"].rstrip().split()[-1][:-1])
	# 		elif field["field"] == "@message" and field["value"].startswith("got_data_time"):
	# 			print("got_data_time: ", field["value"].rstrip().split()[-1][:-1])
	# 		elif field["field"] == "@maxMemoryUsed":
	# 			tokens = field["value"].rstrip().split('E')
	# 			memory = float(tokens[0]) * pow(10, int(tokens[1])) / 1000000
	# 			print("maxMemoryUsed: ", memory)
	# 	if flag == False:
	# 		break




