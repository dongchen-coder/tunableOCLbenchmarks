import sys

f = open('res_3dConvolution.txt','r')

regUsage = ''

for line in f:
	if 'Work group number' in line:
		linetmp = line.replace(',','').split(' ')
		x = linetmp[3]
		y = linetmp[4]
		print x, y, regUsage
	if 'register' in line:
		linetmp = line.split(' ')
		regUsage = linetmp[7]
