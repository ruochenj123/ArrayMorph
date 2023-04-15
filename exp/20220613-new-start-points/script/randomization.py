import random
import sys
import os
import time
import subprocess
import signal
from glob import glob
import argparse

program = "../build/read"
vols = ["s3_vol_get 1", "s3_vol_merge 1",  "s3_vol_range 1", "s3_vol_merge 2", "s3_vol_range 2"]
rows = ["256"]
offsets = ["896 0", "768 512", "512 512", "512 512", "512 512", "512 512", "512 768", "0 896"]
commands = []
for i in range(7):
    n = str(int(rows[-1]) * 2)
    rows.append(n)
for i, r in enumerate(rows):
    rows[i] = r + " " + offsets[i]

def cross_prod_two(set1, set2):
	ret = [];
	for el1 in set1:
		for el2 in set2:
			ret.append([el1, el2])

	return ret

def cross_prod(sets):
	return reduce(cross_prod_two, sets, [[]])

outputfile = sys.argv[1]
logfile = sys.argv[2]

print "Redirecting stdout to \"" + logfile + "\"."
output = open(logfile, "w");

paras = cross_prod_two(vols, rows)
for p in paras:
    commands.append(program + " " + p[0] + " " + p[1])
print(commands)


iterations = 3
secondsbetweenpolls = 15
minutesfortimeout = 300


experiments = [];
for c in commands:
	experiments+=[c] * iterations
random.shuffle(experiments)


totalexp = len(experiments)
counter = 0
for i, c in enumerate(experiments):
	run = c
	print(run)
	counter += 1

	print time.ctime()
	print str(counter) + "/" + str(totalexp), "(" + str(int(float(counter*100)/totalexp)) + "%): Executing \"" + run + "\"."
	sys.stdout.flush()
	print >> output, "#DATE -", time.ctime()
	print >> output, "#####", run
	output.flush()

	#print executable

	oo = open(outputfile, "a+");
	print >> oo, run
	print >> oo, time.ctime()
	handle = subprocess.Popen("( " + run + " )", close_fds=True, preexec_fn=os.setpgrp,
				shell=True, stdout=oo, stderr=subprocess.STDOUT);
	oo.close();

	termin = True
	time.sleep(1)
	for pollround in range(minutesfortimeout*60/secondsbetweenpolls+1):
		if (handle.poll() is not None):
			termin = False
			break;
		time.sleep(secondsbetweenpolls)

	if (termin):
		print "Terminating process due to timeout..."
		sys.stdout.flush();
		os.killpg(handle.pid, signal.SIGTERM)
		handle.wait()
		print >> output, "> Process terminated due to timeout."

	output.flush();
	print >> output, "---- PS INFO START ----"
	f = os.popen('ps -ef | grep -v "^[ ]*root"');
	for line in f:
				 print >> output, line,
	f.close();
	print >> output, "---- PS INFO END ----"
	output.flush()
	os.fsync(output.fileno())

	time.sleep(2);  # cooldown time
output.close();
