import csv
import sys
import query

result = sys.argv[1]
output = sys.argv[2]

re = [['Date','Real Size', 'Transferred Size(MB)', 'Read Size(MB)', 'Returned Size(MB)', 'Total Time','Start Timestamp', 'End Timestamp', 'Billed Duration(ms)']]

q = """
    fields @timestamp, @billedDuration, @memorySize, @maxMemoryUsed, @message
"""
logquery = query.LogQuery()

with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 9
            size = int(c.split(' ')[1])
            r[2] = c.rstrip().split(' ')[-1]
            r[3] = size
            r[4] = c.split(' ')[2]
        elif c.startswith('Tue'):
            r[0] = c[:-1]
        elif c.startswith('transferred size:'):
            r[1] = c.rstrip().split()[-1]
        elif c.startswith('Total'):
            r[5] = c.split(' ')[-1][:-1]
        elif c.startswith('start_stamp'):
            r[6] = c.rstrip().split()[-1]
        elif c.startswith('end_stamp'):
            r[7] = c.rstrip().split()[-1]
            res = logquery.run(r[6], r[7], q)
            flag = False
            for log in res["results"]:
                for field in log:
                    if field["field"] == "@billedDuration":
                        r[-1] = field["value"]
                        flag = True
                        break
                if flag:
                    break
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
