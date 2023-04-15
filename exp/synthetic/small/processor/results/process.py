import csv
import sys
import s3query
import Cost

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Type', 'Query', 'Transferred Size (Byte)','Total Time (s)','Cost ($)', 'Lambda Time (ms)']]
dates = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
s3_q = """
    fields @timestamp, @billedDuration, @message
    |fields toMillis(@timestamp) as millis
"""
s3logquery = s3query.S3LogQuery()
cost = 0
req_num = 0
lambda_num = 0
transfer_size = 0
start_stamp = 0
end_stamp = 0
with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 7
            cs = c.split()
            if cs[1] == "s3_merge":
                r[1] = "MERGE"
            elif cs[1] == "s3_get":
                r[1] = "GET"
            elif cs[1] == "s3_lambda":
                r[1] = "LAMBDA"
            elif cs[1] == "s3_range":
                r[1] = "Multi-fetch"
            else:
                r[1] = "Pareto"
            r[2] = cs[2].split('_')[1]
            cost = 0
            req_num = 0
            lambda_num = 0
            transfer_size = 0
        elif c[:3] in dates:
            r[0] = c[:-1]
        elif c.startswith('Total Time:'):
            r[4] = c.split(' ')[-1][:-1]
            r[3] = transfer_size
            cost += Cost._req * req_num + Cost._lambda_req * lambda_num + Cost._transfer * float(transfer_size) / 1024 / 1024 / 1024
            r[-2] = cost
            re.append(r)
        elif c.startswith('transfer_size:'):
            transfer_size += int(c.rstrip().split()[-1])
        elif c.startswith('LAMBDA:'):
            lambda_num += int(c.rstrip().split()[-1])
        elif c.startswith("s3_req_num:"):
            req_num += int(c.rstrip().split()[-1])
        elif c.startswith('start_stamp'):
            start_stamp = c.rstrip().split()[-1]
        elif c.startswith('end_stamp'):
            end_stamp = c.rstrip().split()[-1]
            lambda_time = 0
            res = s3logquery.run(start_stamp, end_stamp, s3_q)
            for log in res["results"]:
                for field in log:
                    if field["field"] == "@billedDuration":
                        lambda_time += float(field["value"])
            cost += Cost._lambda * lambda_time
            r[-1] = lambda_time

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
