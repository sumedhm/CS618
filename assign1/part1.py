import random
import time
import numpy

exec_time = []
for x in range(1000):
	filename = 'r'+str(random.randint(0,99)).zfill(2)+'.bin'
	start = time.clock()
	fd = open(filename, 'r')
	fd.read()
	fd.close()
	end = time.clock()
	exec_time.append(end - start)

print "Part 1 statistics:"
print "Min - " + str(min(exec_time))
print "Max - " + str(max(exec_time))
print "Average - " + str(numpy.mean(exec_time))
print "Standard deviation - " + str(numpy.std(exec_time))


