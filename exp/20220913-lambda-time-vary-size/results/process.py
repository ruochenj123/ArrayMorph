import csv
import sys
import query

result = sys.argv[1]
output = sys.argv[2]

re = [['Date','# of requests', 'Total Time','Start Timestamp', 'End Timestamp', 'Read Data Time(s)',
        'Write Back Time(s)', 'Billed Duration(ms)']]

q = """
    fields @timestamp, @billedDuration, @memorySize, @maxMemoryUsed, @message
"""
logquery = query.LogQuery()

with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 8
            size = int(c.split(' ')[-1])
            r[1] = size
        elif c.startswith('Tue'):
            r[0] = c[:-1]
        elif c.startswith('Total'):
            r[2] = c.split(' ')[-1][:-1]
        elif c.startswith('start_stamp'):
            r[3] = c.rstrip().split()[-1]
        elif c.startswith('end_stamp'):
            r[4] = c.rstrip().split()[-1]
            res = logquery.run(r[3], r[4], q)
            for log in res["results"]:
                for field in log:
                    if field["field"] == "@billedDuration":
                        r[-1] = field["value"]
                    elif field["field"] == "@message" and field["value"].startswith("write_back_time"):
                        r[-2] = field["value"].rstrip().split()[-1][:-1]
                    elif field["field"] == "@message" and field["value"].startswith("got_data_time"):
                        r[-3] = field["value"].rstrip().split()[-1][:-1]
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
