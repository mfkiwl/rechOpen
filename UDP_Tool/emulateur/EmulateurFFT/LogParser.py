import sys
import numpy as np
from operator import itemgetter, attrgetter
import Image
from math import *

try:
    sys.argv[1]
except NameError:
    startingpoint = 'Missing an arg'
else:
    startingpoint = sys.argv[1]

i = 0
MegaLine = []
Tableau = []
BaseData = []


#with open("Baseline.conf", 'r') as BaselineFile:
#	for line in BaselineFile:
#		line = line.split('\t')
#		del line[-1]
#		BaseData = line
#BaseData = np.array(BaseData).astype(int)

print BaseData
print "\n\n"

with open(startingpoint, 'r') as echOpenLog:
	for line in echOpenLog:
		if (i==4):
			#print MegaLine
			Tableau.append(MegaLine)
			i=0
		if (i==0):
			line = line.split(' ')
			del line[0]
			del line[0]
			del line[0]
			del line[0]
			del line[0]
			del line[0]
			del line[0]
			del line[0]			
			MegaLine = line
		else:
			line = line.split(' ')
			del line[0]
			del line[0]
			del line[0]
			del line[0]
			del line[0]
			del line[0]
			del line[0]
			del line[0]
			MegaLine += line
		i=i+1

Tableau.append(MegaLine)

data = np.array(Tableau).astype(int)
col = 0
i,j = np.unravel_index(data.argmax(), data.shape)
MaxMax = data[i,j]	
print MaxMax

#SortedTable = data[np.argsort(data[:,col])]
SortedTable = data

PointsPerLine = len(SortedTable[0])
NbOfPoints = len(SortedTable)
print PointsPerLine
print NbOfPoints
data = ((data*255)/MaxMax)
data = np.array(data).astype(int)
##work in progress	


size = (NbOfPoints,PointsPerLine)
print size
im = Image.new('RGB',size)
pix = im.load()

for i in range(size[0]):
    for j in range(size[1]):
	value = ((int(data[i][j])))
	SortedTable[i][j] = value
print data
#SortedTable = SortedTable[np.argsort(SortedTable[:,col])]


for i in range(size[0]):
    for j in range(size[1]):
	value = abs(int(SortedTable[i][j]))
        pix[i,j] = (value,value,value) 

outfile = startingpoint +"-fft.png"
im.save(outfile)


sizeFFT = (256,16*1024*2) #
tableFFT = np.zeros(shape = (sizeFFT[0], sizeFFT[1]))
tableFFT = np.array(tableFFT).astype(int)
#print tableFFT
for i in range(sizeFFT[0]):
    for j in range(sizeFFT[1]):
	if (j>1500):
		if (j <(1500+1024)):
        		tableFFT[i][j] = data[i][j-1500]
	else:
		tableFFT[i][j] = 0
print "size:"
print len(tableFFT)
print len(tableFFT[0])

tableFFT = np.array(tableFFT).astype(int)

MyFilteredImage = np.zeros(shape = (sizeFFT[0], sizeFFT[1]))
for i in range(sizeFFT[0]):
	MyFilteredImage[i] = np.fft.irfft(tableFFT[i],sizeFFT[1])


i,j = np.unravel_index(MyFilteredImage.argmax(), MyFilteredImage.shape)
MaxMax = MyFilteredImage[i,j]	
MyFilteredImage = ((MyFilteredImage*255)/MaxMax)
MyFilteredImage = np.array(MyFilteredImage).astype(int)


imt = Image.new('RGB',(sizeFFT[0],sizeFFT[1]))
pix = imt.load()


for i in range(sizeFFT[0]):
    for j in range(sizeFFT[1]):
	value = MyFilteredImage[i][j]
        pix[i,j] = (value,value,value) 

outfile = startingpoint +"-signal.png"
imt.save(outfile)

	
