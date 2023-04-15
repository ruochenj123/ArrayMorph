import sys
import re
result = sys.argv[1]

num = 0
io = 0
s = []
with open(result, encoding="ISO-8859-1") as f:
    content = f.readlines()
    flag = False
    for c in content:
        if c.startswith('content-type: application/xml'):
            flag = True
        if flag and c.startswith('range: bytes='):
            range = re.split('=|-', c[:-1])
            if range not in s:
                num += 1
                io += int(range[-1]) - int(range[-2]) + 1
                s.append(range)
            flag = False
print(num, io)

