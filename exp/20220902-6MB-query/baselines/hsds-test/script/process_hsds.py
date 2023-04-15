import sys
import re
import csv
result = sys.argv[1]
out = sys.argv[2]

logs = [["start", "finish"]]

num = 0
io = 0
s = []
with open(result, encoding="ISO-8859-1") as f:
    content = f.readlines()
    for c in content:
        if "s3Client.get_object" in c:
            num += 1
            io += int(c.split("bytes=")[-1])
            start = float(re.split('start=| finish', c)[1])
            finish = float(re.split('finish=| elapsed', c)[1])
            logs.append([start, finish])
print(num, io)

with open(out, 'w') as csvf:
    csv_writer = csv.writer(csvf, delimiter=',')
    for r in logs:
        csv_writer.writerow(r)
