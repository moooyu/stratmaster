import subprocess
import sys
import re
from pipes import *







class generator:
	def __init__(self,program):# program be a list of list
		self.author = "Ryan"
		
	def generate(self):
		line = [[]*3]
		line[0].append('/*StratMaster HelloWorld*/')
		line[0].append('USE ACCOUNT ac_master;')
		line[1].append({'ALGORITHM al()':'\n{\n}\n'})
		line[1].append({'STRATEGY hello':'{\nWHEN ( al() IS TRUE )\n{\nBUY { WHAT : EQTY(hi).AMOUNT(100).PRICE(99.99); }\n}\n}'})

'''
line = []
n = '20'
sym1 = 'XYZ'
sym2 = 'JOJO'
amt1 = '85'
amt2 = '85'
price1 = '129.99'
price2 = '29.00'

comment = '/*StratMaster HelloWorld*/\n'
line.append(comment)
heads = 'USE ACCOUNT ac_master;\n'
line.append(heads)

body = 'ALGORITHM al()\n\
{\n\
}\n\
\n'
line.append(body)

body = 'STRATEGY hello\n\
{\n\
	WHEN ( al() IS TRUE )\n\
	{\n\
		BUY { WHAT : EQTY(hi).AMOUNT(100).PRICE(99.99); }\n\
	}\n\
}\n'
line.append(body)
'''
with open('testcase','r') as f:
	feed = ['/*<'+x for x in f.read().split('/*<')]



'''
with open('hello1.sm','w') as f:
	for l in line:
		f.write(l)
'''
'''
with open('hello.sh'+n+'_out','w') as f:
	line1 = ' '.join(['BOUGHT',amt1,sym1,price1])
	f.write(line1)
	line2 = ' '.join(['SOLD',amt2,sym2,price2])
	f.write(line2)


STRATEGY hello 
{ 
   BUY { WHAT: EQTY(XYZ).AMOUNT(85).PRICE(129.99);  } 
   SELL { WHAT: EQTY(JOJO).AMOUNT(432).PRICE(29.01); }
}
'''

'''
BOUGHT 85 AAPL 22.00
SOLD 85 AAPL 22.00
subprocess.call("./test.sh")
/dev/null
'''
feed = feed[1:]
fail = []
count = 0
for line in feed[:]:
	
	with open('test.sm','w') as f:f.write(line)
	exe = subprocess.Popen("./strat.out",stdin=open('test.sm','r'), stdout = subprocess.PIPE , stderr=subprocess.PIPE)
	out =  [exe.stdout.read(),exe.stderr.read()]
	res = 'pass'
	if count < 30:
		expected = re.findall('[0-9]*\.?[0-9]+',line)[1:]
		for x in expected:
			if x not in out[0]: 
				res = 'fail'
				break
		if 'syntax' in out[1]:res = 'fail'
	else:
		expected = line.split('\n')[1].strip('*/')
		if expected not in out[1]: res = 'fail'
	print "case",count,':',res
	fail.append([line,out[0]+out[1],expected])
	if res == 'fail':
		print 'fail case number',count
	count += 1
n = int(raw_input('type the fail case by order: '))
print "/*************Source Code**************/"
print fail[n][0]
print "\n/*****************Program Output****************/"
print fail[n][1]
print "\n/*******************Expected Output*****************/"
print fail[n][2]
