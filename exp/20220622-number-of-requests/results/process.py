import csv
import sys

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Object Size (KB)', 'Total Time']]

with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 3
            o = c.rstrip().split()[-1]
            r[1] = int(o) / 1024
        elif c.startswith('Tue'):
            r[0] = c[:-1]
        elif c.startswith('Total'):
            r[2] = c.split(' ')[-1][:-1]
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
