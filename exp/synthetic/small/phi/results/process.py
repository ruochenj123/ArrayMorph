import csv
import sys
import s3query
# import gcquery
import Cost
import time

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Platform', 'Query', 'Phi', 'Transferred Size (Byte)','Total Time (s)','Cost ($)', 'Lambda Time (ms)']]
dates = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
s3_q = """
    fields @timestamp, @billedDuration, @message
    |fields toMillis(@timestamp) as millis
"""
s3logquery = s3query.S3LogQuery()
# gclogquery = gcquery.GCLogQuery()
cost = 0
req_num = 0
lambda_num = 0
transfer_size = 0
start_stamp = 0
end_stamp = 0
lambda_time = 0
with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 8
            cs = c.split()
            vol = cs[1].split('_')[0]
            if (vol == "azure"):
                r[1] = "Azure"
            elif (vol == "s3"):
                r[1] = "S3"
            else:
                r[1] = "GCS"
            r[2] = cs[2].split('_')[1]
            chunk = int(cs[-1].split('_')[-1])
            r[3] = cs[1].split('_')[-1]
            cost = 0
            req_num = 0
            lambda_num = 0
            transfer_size = 0
            lambda_time = 0
        elif c[:3] in dates:
            r[0] = c[:-1]
        elif c.startswith('Total Time:'):
            r[5] = c.split(' ')[-1][:-1]
            r[4] = transfer_size
            cost += Cost._req[r[1]] * req_num + Cost._lambda_req[r[1]] * lambda_num + Cost._transfer[r[1]] * float(transfer_size) / 1024 / 1024 / 1024 + Cost._lambda[r[1]] * lambda_time
            r[-2] = cost
            r[-1] = lambda_time
            re.append(r)
        elif c.startswith('transfer_size:'):
            transfer_size += int(c.rstrip().split()[-1])
        elif c.startswith('LAMBDA:'):
            lambda_num += int(c.rstrip().split()[-1])
        elif c.startswith("s3_req_num:") or c.startswith("azure_req_num:") or c.startswith("gc_req_num:"):
            req_num += int(c.rstrip().split()[-1])
        elif c.startswith('start_stamp'):
            start_stamp = c.rstrip().split()[-1]
        elif c.startswith('end_stamp'):
            end_stamp = c.rstrip().split()[-1]
            if r[1] == "S3":
                res = s3logquery.run(start_stamp, end_stamp, s3_q)
                for log in res["results"]:
                    for field in log:
                        if field["field"] == "@billedDuration":
                            lambda_time += float(field["value"])

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
