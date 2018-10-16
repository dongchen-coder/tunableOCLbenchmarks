from os import listdir
from os.path import isfile, join
mypath = './res'
onlyfiles = [f for f in listdir(mypath) if isfile(join(mypath, f))]

occupency = {}
l2hitRate = {}
for i in [1, 2, 4, 8]:
	occupency[i] = {}
	l2hitRate[i] = {}

for f in onlyfiles:
	files = open(mypath+'/'+f, 'r')
	i = int(f.split('_')[0])
	j = int(f.split('_')[1])
	for line in files:
		if 'achieved_occupancy' in line:
			linetmp = line.replace('\n','').split(" ")
			linetmp = filter(lambda a: a != "", linetmp)
			occupency[i][j] = float(linetmp[-1])
		if 'l2_l1_read_hit_rate' in line:
			linetmp = line.replace('\n','').replace('%','').split(" ")
			linetmp = filter(lambda a: a != "", linetmp)
			l2hitRate[i][j] = float(linetmp[-1])
		

for i in [1, 2, 4, 8]:
	for j in [1, 2, 4, 8, 16, 32]:
		print occupency[i][j],
	print ''

for i in [1, 2, 4, 8]:
	for j in [1, 2, 4, 8, 16, 32]:
		print l2hitRate[i][j],
	print ''


