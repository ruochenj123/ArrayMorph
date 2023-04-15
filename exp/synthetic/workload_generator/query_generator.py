import argparse
import random
import math

parser = argparse.ArgumentParser()
parser.add_argument('-d', dest='dset')
parser.add_argument('-q', dest='query')
parser.add_argument('-n', dest='num', type=int)
parser.add_argument('-s', dest='sel', type=float)




args = parser.parse_args();
print(args.dset)
print(args.query)

num = args.num
sel = args.sel

if (args.dset == "small"):
	x = 2 * 32 * 1024
elif (args.dset == "large"):
	x = 4 * 2 * 32 * 1024

queries = []

random.seed(1)

for i in range(num):
	if (args.query == "row"):
		x1 = random.randint(0, x - (int)(x * sel))
		x2 = x1 + (int)(x * sel) - 1
		queries.append([x1, x2, 0, x - 1])
	elif (args.query == "column"):
		x1 = random.randint(0, x - (int)(x * sel))
		x2 = x1 + (int)(x * sel) - 1
		queries.append([0, x - 1, x1, x2])
	elif (args.query == "cube"):
		c = int(math.sqrt(sel) * x)
		x1 = random.randint(0, x - c - 1)
		y1 = random.randint(0, x - c - 1)
		queries.append([x1, x1 + c - 1, y1, y1 + c -1])



train = args.dset + "_" + args.query + "_train.txt"
test = args.dset + "_" + args.query + "_test.txt"

with open(train, 'w') as f:
	f.write("2\n")
	f.write("{0} {1}\n".format(x, x))
	for i in range(num // 2):
		q = queries[i]
		f.write("{0} {1} {2} {3}\n".format(q[0],q[1],q[2],q[3]))

with open(test, 'w') as f:
	f.write("2\n")
	f.write("{0} {1}\n".format(x, x))
	for i in range(num // 2):
		q = queries[i + num // 2]
		f.write("{0} {1} {2} {3}\n".format(q[0],q[1],q[2],q[3]))



