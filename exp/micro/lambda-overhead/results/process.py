import csv
import sys
import s3query
import gcquery
import time

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Type', '# of lambda','Total Time','Start Timestamp', 'End Timestamp', 'Billed Duration(ms)']]
dates = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
s3_q = """
    fields @timestamp, @billedDuration, @message
    |fields toMillis(@timestamp) as millis
"""
s3logquery = s3query.S3LogQuery()
gclogquery = gcquery.GCLogQuery()
lambda_req = ""
with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 7
            vol = c.split()[1]
            if (vol.startswith('gc')):
                r[1] = "Google"
                lambda_req = "gc_lambda_num"
            else:
                r[1] = "S3"
                lambda_req = "s3_lambda_num"
        elif c[:3] in dates:
            r[0] = c[:-1]
        elif c.startswith(lambda_req):
            print(lambda_req)
            # print(c)
            r[2] = c.rstrip().split(' ')[-1]
        elif c.startswith('Total'):
            r[3] = c.split(' ')[-1][:-1]
        elif c.startswith('start_stamp'):
            r[4] = c.rstrip().split()[-1]
        elif c.startswith('end_stamp'):
            r[5] = c.rstrip().split()[-1]
            billed_time = []
            read_time = []
            print("lambda_num: ", r[2])
            if r[2] == "0":
                r[-1] = 0
                re.append(r)
                continue
            if r[1] == "S3":
                res = s3logquery.run(r[4], r[5], s3_q)
                # flag = True
                for log in res["results"]:
                    for field in log:
                        # if field["field"] == "@message" and "Init Duration" in field["value"]:
                        #     flag = False
                        #     print("cold start")
                        #     break
                        if field["field"] == "@billedDuration":
                            billed_time.append(float(field["value"]))
                        elif field["field"] == "@message" and field["value"].startswith("got_data_time"):
                            read_time.append(float(field["value"].rstrip().split()[-1][:-1]))
                r[-1] = float(sum(billed_time)) / len(billed_time)
                # r[-2] = float(sum(read_time)) / len(read_time)
            else:
                r[-1], _ = gclogquery.query(r[4], r[5])
            time.sleep(5)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
