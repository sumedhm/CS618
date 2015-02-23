import random
import time
import numpy
import os

exec_time = []
for x in range(1000):
	os.system("sync | sudo tee /proc/sys/vm/drop_caches")
	filename = 'r'+str(random.randint(0,99)).zfill(2)+'.bin'
	start = time.clock()
	fd = open(filename, 'wb+')
	for y in range(1024):
		fd.seek(random.randint(0,2046))
		fd.write(str(random.randint(0,9)))
	fd.close()
	end = time.clock()
	exec_time.append(end - start)

print "Part 3 statistics:"
print "Min - " + str(min(exec_time))
print "Max - " + str(max(exec_time))
print "Average - " + str(numpy.mean(exec_time))
print "Standard deviation - " + str(numpy.std(exec_time))


