with open('dinosaur.obj', 'r') as f:
	lines = f.readlines()
	for line in lines:
		if (line[0:2] == 'v '):
			comps = line.split(' ')
			if comps[3][-1] == '\n':
				comps[3] = comps[3][:-1]
			#if comps[0] != '\n':
			print comps[0], comps[3], comps[2], comps[1]
		else:	
			print line
	print len(lines)