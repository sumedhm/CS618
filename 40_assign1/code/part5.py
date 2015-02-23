import random
import time
import numpy
import os

exec_time = []
array = []
z = 100
print "\n\nPart 5 statistics:"
while z<=900:
	os.system("sync | sudo tee /proc/sys/vm/drop_caches")
	for x in range(1000):
		if (x<z):
			c = 'r'
		else:
			c = 's'
		filename = c+str(random.randint(0,99)).zfill(2)+'.bin'
		array.append(filename)
	random.shuffle(array)
	for filename in array:
		start = time.clock()
		fd = open(filename, 'r')
		fd.read()
		fd.close()
		end = time.clock()
		exec_time.append(end - start)
		#print filename
	print "##############   " + str(z/10) + "% r  ##################"
	print "Min - " + str(min(exec_time))
	print "Max - " + str(max(exec_time))
	print "Average - " + str(numpy.mean(exec_time))
	print "Standard deviation - " + str(numpy.std(exec_time))
	array = []
	exec_time = []
	z += 100
	print "\n"



