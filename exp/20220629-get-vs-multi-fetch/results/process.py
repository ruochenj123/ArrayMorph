import csv
import sys

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Type', 'Object Size (KB)', '# of requests', 'Total Time']]

with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 5
            o = c.rstrip().split()[-2]
            r[2] = int(o) / 1024
            vol = c.rstrip().split()[-1]
            if vol == 's3_vol_range':
                r[1] = 'Multi-Fetch'
                r[-2] = 16384
            else:
                r[1] = 'GET'
                r[-2] = 1024 * 1024 /r[2]
        elif c.startswith('Wed'):
            r[0] = c[:-1]
        elif c.startswith('Total'):
            r[-1] = c.split(' ')[-1][:-1]
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
