import csv
import sys

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Type', 'Object Size (KB)', 'Total Time']]

with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 4
            o = c.rstrip().split()
            r[2] = int(o[1]) / 1024
            if o[2].startswith("gc"):
                r[1] = "Google"
            else:
                r[1] = "Azure"
        elif c.startswith('Sat'):
            r[0] = c[:-1]
        elif c.startswith('Total'):
            r[3] = c.split(' ')[-1][:-1]
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
