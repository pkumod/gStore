import os
import re

jena_dir = '/home/chenjiaqi/jena/bin/'
gstore_dir = '/home/chenjiaqi/gStore-master/'
devgstore_dir = '/home/chenjiaqi/devGstore/'
query_dir = '/home/chenjiaqi/useful/allquery'

if __name__ == '__main__':
	gstore_nores = open('gstore.nores', 'w')

	query_set = open(query_dir, 'r').read().split('myownlinebreak')	
	for query in query_set:
		output = open('query', 'w')
		output.write(query + '\n')
		output.close()

		os.system(jena_dir + 'tdbquery --results TSV --loc ' + jena_dir + 'DBpedia --query query >& jena.ans')
		os.system(devgstore_dir + 'bin/gquery ' + devgstore_dir + 'DBpedia query >& gstore.ans')
		
		jans = open('jena.ans').read().split('\n')[0 : -1]

		gans = open('gstore.ans').read()
		if gans.find('final result') == -1:
			gstore_nores.write(query + '\nmyownlinebreak\n')
			continue

		gans = gans.split('\n')
		for i in range(0, len(gans)):
			if gans[i].find('final result') != -1:
				gans = gans[i + 1 : -2]
				break

		vars = {}
		jans_map = []
		for v in jans[0].split('\t'):
			if v not in vars:
				vars[v] = len(vars)
			jans_map.append(vars[v])
		gans_map = []
		for v in gans[0].split('\t'):
			if v not in vars:
				vars[v] = len(vars)
			gans_map.append(vars[v])

		var_num = len(vars)
		jans_text = []
		for i in range(1, len(jans)):
			list = [''] * var_num
			values = jans[i].split('\t')
			if len(values) == var_num:
				for j in range(0, len(values)):
					if len(values[j]) == 0:
						list[jans_map[j]] = 'Null'
					else:
						list[jans_map[j]] = values[j]
				jans_text.append('\t'.join(list))
		gans_text = []
		for i in range(1, len(gans)):
			list = [''] * var_num
			values = gans[i].split('\t')
			if len(values) == var_num:
				for j in range(0, len(values)):
					if len(values[j]) == 0:
						list[gans_map[j]] = 'Null'
					else:
						list[gans_map[j]] = values[j]
				gans_text.append('\t'.join(list))

		jans_text.sort()
		gans_text.sort()

		equal_flag = True
		if len(jans_text) != len(gans_text):
			equal_flag = False
		else:
			for i in range(0, len(jans_text)):
				if jans_text[i] != gans_text[i]:
					equal_flag = False
					break

		if equal_flag:
			print 'equal'
			print query
			print 'the answer of jena is : '
			for text in jans_text:
				print text
			print 'the answer of gstore is : '
			for text in gans_text:
				print text
		else:
			print 'not equal'
			print query
			print 'the answer of jena is : '
			for text in jans_text:
				print text
			print 'the answer of gstore is : '
			for text in gans_text:
				print text

	gstore_nores.close()
