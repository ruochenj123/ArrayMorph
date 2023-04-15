from google.cloud import logging
from datetime import datetime, timedelta
import time
import os
import pytz

class GCLogQuery:
	def __init__(self):
		# self.client = logging.Client.from_service_account_json(cred_file)
		self.client = logging.Client(project="keen-honor-376001")
	def query(self, start_time, end_time):
		start = datetime.fromtimestamp(int(start_time) / 1000.0).astimezone(pytz.utc)
		end = datetime.fromtimestamp(int(end_time) / 1000.0).astimezone(pytz.utc)
		filter = 'resource.type="cloud_function" \
			resource.labels.function_name="hyperslab" \
			resource.labels.region="us-east1" \
			logName="projects/keen-honor-376001/logs/cloudfunctions.googleapis.com%2Fcloud-functions" '
		filter += 'timestamp >= {0} AND timestamp <= {1}'.format(
				"\"" + start.replace(microsecond=0).isoformat() + "\"",
				"\"" + end.replace(microsecond=0).isoformat() + "\""
			)
		print(filter)
		entries = self.client.list_entries(filter_=filter)
		exec_time = []
		read_time = []
		for entry in entries:
			message = entry.payload
			if message.startswith("Function execution took"):
				exec_time.append(int(message.split()[3]))
			elif message.startswith("got_data_time:"):
				read_time.append(float(message.split()[1]))
		print(len(exec_time))
		print(len(read_time))
		return float(sum(exec_time)) / len(exec_time), sum(read_time) / len(read_time) 

if __name__ == '__main__':
	gcquery = GCLogQuery()
	a, b = gcquery.query(1675715265865, 1675715322473)
	
	print(a)
	print(b)