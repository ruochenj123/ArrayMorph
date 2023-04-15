import csv
import sys
# import query
import re

result = sys.argv[1]
output = sys.argv[2]

logs = [['Date','System', 'IO Num', 'IO Size', 'Time']]

with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('python3'):
            r = [None] * 5
            command = c.split()[1]
            s = re.split('\.|\/', command)[-2]
            if (s == 'hybrid'):
                r[1] = 'Pareto'
                size = 0
                io_num = 0
            elif (s == 'hsds'):
                r[1] = "HSDS"
                size = 66804337373
                io_num = 22852
            else:
                r[1] = "TileDB"
                size = 609622288 * 10
                io_num = 159 * 10
        elif c.startswith('Wed'):
            r[0] = c[:-1]
        elif c.startswith('transferred size:'):
            size  += int(c.rstrip().split()[-1])
        elif c.startswith('# of requests'):
            io_num += int(c.rstrip().split()[-1])
        elif c.startswith('total time:'):
            r[4] = c.split(' ')[-1][:-1]
            r[2] = io_num
            r[3] = size
            logs.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in logs:
        csv_writer.writerow(r)
