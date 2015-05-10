#!/usr/bin/python

import subprocess
import sys
import re
import argparse
from pipes import *


result = ["85 shares of ZBRA",
	"85 shares of ZBRA",
	"85 shares of ZBRA",
	"Account not found",
	"85 shares of ZBRA",
	"error opening data file",
	"85 shares of ZBRA",
	"error opening data file",
	"error opening account",	#9
	"error",
	"error",
	"85 shares of",
	"85 shares of",
	"85 shares of",
	"85 shares of",
	"85 shares of",
	"85 shares of",
	"85 shares of",
	"85 shares of",
	"syntax error",			#20
	"85 shares of",	
	"85 shares of",
	"85 shares of",
	"85 shares of",
	"already in use",
	"not found",
	"85 shares of",
	"85 shares of",
	"invalid",
	"invalid",
	"85 shares of",
	"85 shares of",
	"85 shares of",
	"shares",			#34
	"syntax error",
	"syntax error"
	"shares of",
	"shares of"			#38
	]
	
	
	
	
	

parser = argparse.ArgumentParser(prog='PLT FINAL',description=
                "python test.py or python test.py --case number")
parser.add_argument('--case',  help='choose a case number')
args = parser.parse_args()



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

s,e = 0,len(feed)
if args.case != None:  s = int(args.case)-1; e = s+1
info = []
count = 0
fail = []
for line in feed[s:e]:
	with open('test.sm','w') as f:f.write(line)
	exe = subprocess.Popen("../devel/strat.out",stdin=open('test.sm','r'), stdout = subprocess.PIPE , stderr=subprocess.PIPE)
	out =  [exe.stdout.read(),exe.stderr.read()]
	res = 'pass'
		#expected = re.findall('[0-9]*\.?[0-9]+',line)[1:]
	if result[count] not in out[0] and result[count] not in out[1]: 
		res = 'fail'

	
	info.append([line,out[0]+out[1],result[count]])
	if res == 'pass': print "case",count+1,':',info[count][0].split('*/')[0],'\n','------->',res
	elif res == 'fail':
		fail.append(count)
		print "case",count+1,':',info[count][0].split('*/')[0],'\n','------->',res
	count += 1
print "/*************summary**************/"
print 'pass: ',count + 1 - len(fail),'/',count+1
for i in fail:
	print 'fail case',i+1,': ',info[i][0].split('*/')[0]
n = int(raw_input('type the fail case number: '))-1
print "/*************Source Code**************/"
print info[n][0]
print "\n/*****************Program Output****************/"
print info[n][1]
print "\n/*******************Expected Output*****************/"
print info[n][2]
if raw_input('dump to log file in this dir?(y/n)') == 'y':
	with open('log','w') as f: f.write(info[n][0])
