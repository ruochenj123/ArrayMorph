import csv
import sys
import math

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Chunking', 'Type', 'Row Size', 'Starting Point', '# of request', 'I/O Size (Bytes)', 'Total Time (s)']]
with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 8
            vol = c.rstrip().split()[1]
            row = int(c.rstrip().split()[-3])
            col = 32768 * 256 / row;
            r[3] = row
            x = int(c.rstrip().split()[-2])
            y = int(c.rstrip().split()[-1])
            xn = min((x + row) / 1024 - x / 1024 + 1, 32)
            yn = min((y + col) / 1024 - y / 1024 + 1, 32)
            r[4] = "[{0}, {1}]".format(x, y)
            vol = vol.split('_')[2]
            if vol == 'get':
                r[2] = 'One GET Per Chunk'
            elif vol == 'range':
                r[2] = 'Multiple RANGE per Chunk'
            else:
                r[2] = 'One RANGE per Chunk'
            if c.rstrip().split()[2] == '1':
                r[1] = 'Yes'
            else:
                r[1] = "No"
            if (vol == 'range'):
                r[-2] = 32 * 1024 * 1024
                r[-3] = row
                if r[1] == 'Yes':
                    r[-3] *= yn
            elif (vol == 'merge'):
                if r[1] == 'Yes':
                    r[-3] = xn * yn
                    r[-2] = yn * 1024 * row * 4
                else:
                    r[-3] = 1
                    r[-2] = ((row - 1) * 32768 + col) * 4
            else:
                r[-3] = xn * yn
                r[-2] = r[-3] * 1024 * 1024 * 4
        elif c.startswith('Fri') or c.startswith('Wed'):
            r[0] = c[:-1]
        elif c.startswith('Total'):
            r[-1] = c.split(' ')[-1][:-1]
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
        

