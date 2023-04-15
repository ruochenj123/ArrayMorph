import csv
import sys
import time

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Type', '# of range','Total Time']]
dates = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]

with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 4
            vol = c.split()[1]
            ratio = float(vol.split('_')[-1])
            r[2] = int(ratio / 100.0 * 256)
            if (vol.startswith('gc')):
                r[1] = "Google"
            elif vol.startswith('s3'):
                r[1] = "S3"
            else:
                r[1] = "Azure"
        elif c[:3] in dates:
            r[0] = c[:-1]
        elif c.startswith('Total'):
            r[3] = c.split(' ')[-1][:-1]
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
