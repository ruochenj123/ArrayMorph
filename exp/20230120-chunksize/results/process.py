import csv
import sys

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Type', '# of requests', 'Total Time']]
date = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 4
            o = c.rstrip().split()
            x = int(o[1])
            r[2] = x * x / 2 / 512 / 512
            if o[2].startswith("gc"):
                r[1] = "Google"
            elif o[2].startswith("azure"):
                r[1] = "Azure"
            else:
                r[1] = "S3"
        elif c[0:3] in date:
            r[0] = c[:-1]
        elif c.startswith('Total'):
            r[3] = c.split(' ')[-1][:-1]
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
