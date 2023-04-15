import csv
import sys
import Cost

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Type', 'Selectivity' , 'Platform','Total Time (s)','Cost ($)', "Transfer Size (bytes)"]]
dates = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
num = {"0.01": 320, "0.05": 1620, "0.1": 3240}

# req_num = 17
# transfer_size = 447201903
req_num = 5
transfer_size = 4730400
t = 0

r = [None] * 7
with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('python3'):
            r[-3] = t * 10
            re.append(r)
            r = [None] * 7
            w = c.split()[3]
            r[3] = c.split()[2]
            r[1] = "TileDB"
            r[2] = w.split('/')[-2]
            r[-1] = transfer_size * num[r[2]]
            r[-2] = num[r[2]] * (transfer_size / 1024 / 1024 / 1024 * Cost._transfer[r[3]] + req_num * Cost._req[r[3]])
            t = 0
        elif c[:3] in dates:
            r[0] = c[:-1]
        elif c.startswith('total time:'):
            t += float(c.split(' ')[-1][:-1])
r[-3] = t * 10
#r[-2] = num[r[2]] * (transfer_size / 1024 / 1024 / 1024 * Cost._transfer[r[3]] + req_num * Cost._req[r[3]])
re.append(r)
with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)