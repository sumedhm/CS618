import random
import time
import numpy

exec_time = []
array = []
z = 0
print "\n\nPart 6 statistics:"
while z<=1000:
	for x in range(1000):
		if x<z:
			c='r'
			n=1024
		else:
			c='s'
			n=32767
		filename = c+str(random.randint(0,99)).zfill(2)+'.bin'
		array.append(filename)
	random.shuffle(array)
	for filename in array:
		start = time.clock()
		fd = open(filename, 'wb+')
		for y in range(n):
			fd.seek(random.randint(0,2*n-2))
			fd.write(str(random.randint(0,9)))
		fd.close()
		end = time.clock()
		exec_time.append(end - start)
	print "##############   " + str(z/10) + "% r  ##################"
	print "Min - " + str(min(exec_time))
	print "Max - " + str(max(exec_time))
	print "Average - " + str(numpy.mean(exec_time))
	print "Standard deviation - " + str(numpy.std(exec_time))
	array = []
	exec_time = []
	z += 100
	print "\n"
	
	
