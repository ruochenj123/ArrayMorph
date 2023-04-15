import csv
import sys

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Chunk Size', 'Type', 'Total Time', 'Transferred Size (bytes)','Start Timestamp', 'End Timestamp', '# of Lambda request', '# of request']]

with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 9
            r[-1] = 0
            o = c.rstrip().split()[-2]
            if (o == "s3_vol_get"):
                r[2] = "GET"
            else:
                r[2] = "Hybrid"
            chunk_size = c.rstrip().split()[-1]
            r[1] = 4
        elif c.startswith('Fri'):
            r[0] = c[:-1]
        elif c.startswith('Total'):
            r[3] = c.split(' ')[-1][:-1]
        elif c.startswith('start_stamp'):
            r[-4] = c.rstrip().split()[-1]
        elif c.startswith('# of lambda'):
            r[-2] = c.rstrip().split()[-1]
        elif c.startswith('# of request'):
            r[-1] = c.rstrip().split()[-1]
        elif c.startswith('transferred size:'):
            r[4] = c.rstrip().split()[-1]
        elif c.startswith('end_stamp'):
            r[-3] = c.rstrip().split()[-1]
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
