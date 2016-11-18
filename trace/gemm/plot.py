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
	tmpInterDz = []
	for j in y:
		tmpIntraCz.append(intraC[i][j])
		tmpInterCz.append(interC[i][j])
		tmpIntraDz.append(intraD[i][j])
		tmpInterDz.append(interD[i][j])
		print interD[i][j],
	print ''
	intraCz.append(tmpIntraCz)
	interCz.append(tmpInterCz)
	intraDz.append(tmpIntraDz)
	interDz.append(tmpInterDz)
'''	
x = [1,2,3,4,5,6,7,8,9,10]
y = [1,2,3,4,5,6,7,8,9,10]

x, y = np.meshgrid(x, y)

fig = plt.figure()
ax = fig.gca(projection='3d')
surf = ax.plot_surface(x, y, intraDz)

ax.set_zlim(0, 1)

ax.set_xscale('log')
ax.set_yscale('log')

print x
print y
print interDz

plt.show()
'''


