import sys
import re
result = sys.argv[1]

num = 0
io = 0
requests = []
with open(result, encoding="ISO-8859-1") as f:
    content = f.readlines()
    for c in content:
        if "s3Client.get_object" in c:
            io += int(c.split('=')[-1])
            num += 1
print(num, io)

