import sys
import numpy as np
import matplotlib.pyplot as plt
#from mpl_toolkits.mplot3d import axes3d

f = open('res.txt', 'r')

intraC = {}
interC = {}
intraD = {}
interD = {}

gidx = 0
gidy = 0

for line in f:
	linetmp = line.replace('\n','').replace(',','').split(' ')
	linetmp = filter(lambda a: a != '', linetmp)
	#print linetmp
	if linetmp[0] == 'global':
		gidx = int(linetmp[3])
		gidy = int(linetmp[4])
		if gidx not in intraC.keys():
			intraC[gidx] = {}
			interC[gidx] = {}
			intraD[gidx] = {}
			interD[gidx] = {}
	if linetmp[0] == 'Intra':
		intraC[gidx][gidy] = int(linetmp[5])
		interC[gidx][gidy] = int(linetmp[6])
	if linetmp[0] == 'distribution':
		intraD[gidx][gidy] = float(linetmp[1])
		interD[gidx][gidy] = float(linetmp[2])
	
x = intraC.keys()
y = intraC[gidx].keys()
x.sort()
y.sort()

intraCz = []
interCz = []
intraDz = []
interDz = []
for i in x:
	tmpIntraCz = []
	tmpInterCz = []
	tmpIntraDz = []
	tmpinterDz = []
	for j in y:
		tmpIntraCz.append(intraC[i][j])
		tmpInterCz.append(interC[i][j])
		tmpIntraDz.append(intraD[i][j])
		tmpInterDz.append(interD[i][j])
	intraCz.append(tmpIntraCz)
	interCz.append(tmpInterCz)
	intraDz.append(tmpIntraDz)
	interDz.append(tmpInterDz)
	
#fig = plt.figure()
#surf = ax.plot_surface(x, y, intraCz, rstride=1, cstride=1, cmap=cm.coolwarm, linewidth=0, antialiased=False)

#plt.show()



