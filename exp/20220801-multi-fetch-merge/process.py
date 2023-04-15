import csv
import sys

result = sys.argv[1]
output = sys.argv[2]

re = [['Date', 'Chunk Size (KB)', 'Type', 'Total Time', '# of request']]

with open(result) as f:
    content = f.readlines()
    for c in content:
        if c.startswith('../build'):
            r = [None] * 5
            o = c.rstrip().split()[-2]
            if (o == "s3_vol_merge"):
                r[2] = "Merge"
            elif (o == "s3_vol_range"):
                r[2] = "Multi-fetch"
            else:
                r[2] = "GET"
            chunk_size = c.rstrip().split()[-1]
            r[1] = int(chunk_size) / 1024
        elif c.startswith('Mon'):
            r[0] = c[:-1]
        elif c.startswith('Total'):
            r[3] = c.split(' ')[-1][:-1]
        elif c.startswith('# of requests'):
            r[-1] = c.rstrip().split()[-1]
            re.append(r)

with open(output, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in re:
        csv_writer.writerow(r)
