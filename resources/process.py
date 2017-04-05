with open('horse.obj', 'r') as f:
	lines = f.readlines()
	for line in lines:
		if (line[0:2] == 'v '):
			comps = line[:-1].split(' ')
			print comps[0], comps[1], comps[3], comps[2]
		else:
			print line
	print len(lines)