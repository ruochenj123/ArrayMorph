import csv
import sys
import Cost

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Type', 'Selectivity', 'Transferred Size (Byte)','Total Time (s)','Cost ($)']]
dates = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]

req_num = 0
t = 0
transfer_size = 0
r = [None] * 6
p = "s3"
with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('python3'):
            r[-2] = t
            r[3] = transfer_size
            r[-1] = float(r[3]) / 1024 / 1024 / 1024 * Cost._transfer[p] + req_num * Cost._req[p]
            re.append(r)
            r = [None] * 6
            w = c.split()[2]
            r[1] = "Pareto"
            r[2] = w.split('/')[-2]
            req_num = 0
            transfer_size = 0
            t = 0
        elif c[:3] in dates:
            r[0] = c[:-1]
        elif c.startswith('Total Time:'):
            t += float(c.split(' ')[-1][:-1])
        elif c.startswith('transfer_size:'):
            transfer_size += int(c.rstrip().split()[-1])
        elif c.startswith("azure_req_num:"):
            req_num += int(c.rstrip().split()[-1])
r[-2] = t
r[3] = transfer_size
r[-1] = float(r[3]) / 1024 / 1024 / 1024 * Cost._transfer[p] + req_num * Cost._req[p]
re.append(r)
with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
