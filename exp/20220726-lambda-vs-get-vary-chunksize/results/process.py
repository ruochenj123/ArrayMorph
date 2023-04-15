import csv
import sys

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Chunk Size (KB)', 'Type', 'Total Time', 'Start Timestamp', 'End Timestamp', '# of Lambda request']]

with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 7
            r[-1] = 0
            o = c.rstrip().split()[-2]
            if (o == "s3_vol_lambda"):
                r[2] = "Lambda"
            else:
                r[2] = "GET"
            chunk_size = c.rstrip().split()[-1]
            r[1] = int(chunk_size) / 1024
        elif c.startswith('Mon'):
            r[0] = c[:-1]
        elif c.startswith('Total'):
            r[3] = c.split(' ')[-1][:-1]
        elif c.startswith('start_stamp'):
            r[-3] = c.rstrip().split()[-1]
        elif c.startswith('# of lambda'):
            r[-1] = c.rstrip().split()[-1]
        elif c.startswith('end_stamp'):
            r[-2] = c.rstrip().split()[-1]
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
