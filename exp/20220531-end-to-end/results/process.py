import csv
import sys
import math

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Executor', 'Type', 'Row Size', '# of request', 'I/O Size (Bytes)', 'Total Time (s)']]
with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 7
            vol = c.rstrip().split()[1]
            row = int(c.rstrip().split()[-1])
            col = 32768 * 256 / row;
            r[3] = row
            r[2] = vol.split('_')[2]
            if vol[-1] == '1':
                r[1] = 'Pooled'
            else:
                r[1] = 'Default'
            if (r[2] == 'range'):
                r[-3] = row
                r[-2] = 32 * 1024 * 1024
            elif (r[2] == 'merge'):
                r[-3] = 1
                r[-2] = ((row - 1) * 32768 + col) * 4
            else:
                r[-3] = math.ceil(row / 1024) * math.ceil(col / 1024)
                r[-2] = r[-3] * 1024 * 1024 * 4
        elif c.startswith('Tue') or c.startswith('Wed'):
            r[0] = c[:-1]
        elif c.startswith('Total'):
            r[-1] = c.split(' ')[-1][:-1]
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
        

