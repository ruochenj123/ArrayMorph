import csv
import sys
import Cost

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Type', 'Selectivity' ,'Total Time (s)','Cost ($)']]
dates = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]

req_num = {"0.01": 30, "0.05": 35, "0.1": 38}
t = 0
transfer_size = {"0.01": 31870712, "0.05": 97943272, "0.1": 141849056}
r = [None] * 5
p = result.split('.')[0].split('-')[1]
with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('python3'):
            r[3] = t
            re.append(r)
            r = [None] * 5
            w = c.split()[2]
            r[1] = "TileDB"
            r[2] = w.split('/')[-2]
            r[-1] = 250 * transfer_size[r[2]] / 1024 / 1024 / 1024 * Cost._transfer[p] + 250*req_num[r[2]] * Cost._req[p]
            t = 0
        elif c[:3] in dates:
            r[0] = c[:-1]
        elif c.startswith('total time:'):
            t += float(c.split(' ')[-1][:-1])
r[-2] = t
r[-1] = 250 * transfer_size[r[2]] / 1024 / 1024 / 1024 * Cost._transfer[p] + 250 *req_num[r[2]] * Cost._req[p]
re.append(r)
with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
