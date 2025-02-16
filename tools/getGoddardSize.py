import sys, os

path = "build/" + sys.argv[2] + "_" + sys.argv[3] + "/goddard.txt"

with open(sys.argv[1]) as f:
	for line in f:
		if "GODDARD_SIZE" in line:
			tokens=line.split()
			with open(path, "w+") as f:
				sz = int(tokens[0], 16)
				sz += 16
				sz &= 0xFFFFFFF0
				f.write("GODDARD_SIZE = 0x%X;" % sz)
				
