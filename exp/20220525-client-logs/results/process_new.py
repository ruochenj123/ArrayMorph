import sys
import os
import csv
import re

out = [["time"]]
with open("out-new.txt", "r") as f:
	content = f.readlines()
	for i,c in enumerate(content):
		if c.startswith("finish: "):
			t = int(c.split()[-1])
			out.append([t])

with open("submit_time_new.csv", "w") as f:
	csv_writer = csv.writer(f, delimiter=',')
	for r in out:
		csv_writer.writerow(r)
