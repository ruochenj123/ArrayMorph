import csv
import sys

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Platform', 'Type', 'Total Time', 'Transfer size (bytes)', '# of requests']]
date = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 6
            vol = c.rstrip().split()[-1]
            r[1] = vol.split('_')[0]
            r[2] = vol.split('_')[1]
        elif c[0:3] in date:
            r[0] = c[:-1]
        elif c.startswith('transfer_size'):
            r[4] = c.split()[-1]
        elif c.startswith(r[1] + '_req_num'):
            r[-1] = c.split()[-1]
        elif c.startswith('Total'):
            r[3] = c.split(' ')[-1][:-1]
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
