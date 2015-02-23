#!/usr/bin/python

import random

#r00-r99.bin files, size 2kB

for i in range(100):
	filename = open('r'+str(i).zfill(2)+'.bin', 'w')
	randstr = str(random.randint(0,9))
	for j in range(2046):
		randstr += str(random.randint(0,9))
	filename.write(randstr)
	filename.close()

#s00-s99.bin files, size 100kB

for i in range(100):
        filename = open('s'+str(i).zfill(2)+'.bin', 'w')
        randstr = str(random.randint(0,9))
        for j in range(65535):
                randstr += str(random.randint(0,9))
        filename.write(randstr)
        filename.close()

