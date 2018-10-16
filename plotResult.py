import sys
import os
import matplotlib.pyplot.plot as plt

def timeToFloat(t):
	if ("ms" in t):
		return float(t.replace("ms",""))
	print t
	return t

path = "/localdisk/dchenTmp/scheduling/tunableOCLbenchmarks/result"

benches = ["2mm"]
kernels = {"2mm" : ["mm2_kernel1", "mm2_kernel2"]}

for name in benches:
	for kernel in kernels[name]:
		times = []
		for file in os.listdir(path):
			if (name in file):
				f = open(path + "/" + file, "r")
				tmp = 0
				cnt = 0
				for line in f:
					if (kernel in line):
						cnt += 1
						tmp += timeToFloat(line.split()[1])
				tmp = tmp / cnt
				times.append(tmp)
		plt.plot(times)

plt.show()



#print file.replace(name + ".exe_result", "").replace(".txt","").split("_")
			
