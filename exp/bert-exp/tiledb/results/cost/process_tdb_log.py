import sys
import re
result = sys.argv[1]

num = 0
io = 0
requests = []
with open(result, encoding="ISO-8859-1") as f:
    content = f.readlines()
    flag = False
    for c in content:
        if c.startswith("amz-sdk-invocation-id:"):
            rid = c.split(':')[-1]
            if rid not in requests:
                flag = True
                requests.append(rid)
        if flag and c.startswith('range: bytes='):
            range = re.split('=|-', c[:-1])
            num += 1
            io += int(range[-1]) - int(range[-2]) + 1
            flag = False
print(num, io)

