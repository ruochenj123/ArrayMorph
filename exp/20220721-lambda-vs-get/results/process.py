import csv
import sys

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Type', 'Total Time', 'Start Timestamp', 'End Timestamp']]

with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 5
            o = c.rstrip().split()[-1]
            if (o == "s3_vol_lambda"):
                r[1] = "Lambda"
            else:
                r[1] = "GET"
        elif c.startswith('Thu'):
            r[0] = c[:-1]
        elif c.startswith('Total'):
            r[2] = c.split(' ')[-1][:-1]
        elif c.startswith('start_stamp'):
            r[-2] = c.rstrip().split()[-1]
        elif c.startswith('end_stamp'):
            r[-1] = c.rstrip().split()[-1]
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
