import csv
import sys
import query

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Chunk Size(MB)','Lambda Ratio(%)', '# of Lambda', 'Total Time','Start Timestamp', 'End Timestamp',
'Read Data Time(s)', 'Write Back Time(s)', 'Billed Duration(ms)']]

q = """
    fields @timestamp, @billedDuration, @memorySize, @maxMemoryUsed, @message
"""
logquery = query.LogQuery()

with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 10
            r[2] = c.rstrip().split('_')[-1]
            r[1] = 16
        elif c.startswith('Tue'):
            r[0] = c[:-1]
        elif c.startswith('# of lambda:'):
            r[3] = c.rstrip().split()[-1]
        elif c.startswith('Total'):
            r[4] = c.split(' ')[-1][:-1]
        elif c.startswith('start_stamp'):
            r[5] = c.rstrip().split()[-1]
        elif c.startswith('end_stamp'):
            r[6] = c.rstrip().split()[-1]
            r[7] = 0
            r[8] = 0
            r[9] = 0
            if (r[2] != '0'):
                res = logquery.run(r[5], r[6], q)
                for log in res["results"]:
                    for field in log:
                        if field["field"] == "@billedDuration":
                            r[9] += float(field["value"])
                        elif field["field"] == "@message" and field["value"].startswith("write_back_time"):
                            r[8] += float(field["value"].rstrip().split()[-1][:-1])
                        elif field["field"] == "@message" and field["value"].startswith("got_data_time"):
                            r[7] += float(field["value"].rstrip().split()[-1][:-1])
                r[9] /= int(r[3])
                r[8] /= int(r[3])
                r[7] /= int(r[3])
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
