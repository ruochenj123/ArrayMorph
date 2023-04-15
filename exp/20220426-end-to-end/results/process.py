import csv
import sys

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Type', 'Local computation', 'Row Size', '# of request', 'I/O Size (Byte)', 'Total Time (s)']]
with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 7
            vol = c.rstrip().split()[1]
            row = int(c.rstrip().split()[-1])
            col = 32768 * 256 / row;
            r[3] = row
            r[1] = vol.split('_')[2]
            if (vol.split('_')[-1] == "1"):
                r[2] = 'No'
            else:
                r[2] = 'Yes'
            if (r[1] == 'range'):
                r[-3] = row
            elif (r[1] == 'merge'):
                r[-3] = 1
        elif c.startswith('Tue') or c.startswith('Wed'):
            r[0] = c[:-1]
        elif c.startswith('Total'):
            r[-1] = c.split(' ')[-1][:-1]
        elif c.startswith('IO'):
            r[-2] = c.split(' ')[-2]
            if (r[1] == 'get'):
                r[-3] = (int(r[-2])) / 1024 / 1024 / 4
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
        

