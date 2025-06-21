import boto3
from google.cloud import logging
from azure.monitor.query import LogsQueryClient
from azure.identity import DefaultAzureCredential
from datetime import datetime, timedelta
import time
import os

class S3LogQuery:
	def __init__(self):
		self.client = boto3.client('logs',
			region_name = "us-east-2",
		    aws_access_key_id=os.getenv("AWS_ACCESS_KEY_ID"),
		    aws_secret_access_key=os.getenv("AWS_SECRET_ACCESS_KEY"),
		)
		self.log_group = os.getenv("AWS_LAMBDA_LOG_GROUP")
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

def google_log_query(start, end):
    client = logging.Client().from_service_account_json(os.getenv("GOOGLE_CLOUD_STORAGE_JSON"))
    query = f"""
        resource.type="cloud_function"
		resource.labels.function_name="sci4dda-vol-on-ood"
        timestamp >= "{start}"
        timestamp <= "{end}"
    """
    entries = client.list_entries(filter_=query)
    
    for entry in entries:
        if "execution_time" in entry.payload:
            return float(entry.payload["execution_time"])

    return None

def azure_log_query(start, end):
    connection_string = os.getenv("AZURE_CONNECTION_STRING")
    client = LogsQueryClient.from_connection_string(connection_string)

    workspace_id = os.getenv("AZURE_LOG_WORKSPACE_ID")
    query = f"""
        AzureDiagnostics
        | where TimeGenerated between (datetime({start}) .. datetime({end}))
        | where ResourceType == "FUNCTIONAPP"
		| where FunctionName_s == "sci4dda-vol-on-ood"
        | project duration_s
    """

    response = client.query_workspace(workspace_id, query)
    if response.tables:
        for row in response.tables[0].rows:
            return float(row[0]) * 1000 

    return None


if __name__ == '__main__':
	query = """
		fields @timestamp, @billedDuration, @memorySize, @maxMemoryUsed, @message
		|fields toMillis(@timestamp) as millis
	"""
	start = "1662667222794"
	end = "1662667223539"
	logquery = LogQuery()
	res = logquery.run(start, end, query)
	flag = True
	print(res["results"])
	for log in res["results"]:
		for field in log:
			if field["field"] == "@message" and "Init Duration" in field["value"]:
				flag = False
				print("cold start")
				break
			if field["field"] == "@billedDuration":
				print("billedDuration: ", field["value"])
			elif field["field"] == "@message" and field["value"].startswith("write_back_time"):
				print("write_back_time: ", field["value"].rstrip().split()[-1][:-1])
			elif field["field"] == "@message" and field["value"].startswith("got_data_time"):
				print("got_data_time: ", field["value"].rstrip().split()[-1][:-1])
			elif field["field"] == "@maxMemoryUsed":
				tokens = field["value"].rstrip().split('E')
				memory = float(tokens[0]) * pow(10, int(tokens[1])) / 1000000
				print("maxMemoryUsed: ", memory)
		if flag == False:
			break




