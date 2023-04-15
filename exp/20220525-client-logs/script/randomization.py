import random
import sys
import os
import time
import subprocess
import signal
from glob import glob
import argparse

program = "../build/read"
vols = ["s3_vol_range"]
rows = ["4096", "32768"]
commands = []

def cross_prod_two(set1, set2):
	ret = [];
	for el1 in set1:
		for el2 in set2:
			ret.append([el1, el2])

	return ret

def cross_prod(sets):
	return reduce(cross_prod_two, sets, [[]])
paras = cross_prod_two(vols, rows)
for p in paras:
    commands.append(program + " " + p[0] + " " + p[1])
print(commands)

outputfile = sys.argv[1]
logfile = sys.argv[2]

print "Redirecting stdout to \"" + logfile + "\"."
output = open(logfile, "w");


iterations = 1
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

