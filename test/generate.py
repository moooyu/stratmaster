key1 = 'STRATEGY'
sname = 'helloworld'
sym1 = 'apple tree'
amount = str(100000000)
price = str(1.00)
line = "{ WHAT: EQTY("+sym1+").AMOUNT("+amount+").PRICE("+price+");  } "
line1 = "BUY "+line
line2 = "SELL"+line
with open('testcase','a') as f:
	f.write('//<case 3> No\n')
    f.write("USE ACCOUNT my father's account")
	f.write(key1 +' '+ sname + '\n')
	f.write('{\n')
	f.write(line1+'\n')
	f.write(line2+'\n')
	f.write('}\n')

'''
STRATEGY hello 
{ 
   BUY { WHAT: EQTY(XYZ).AMOUNT(85).PRICE(129.99);  } 
   SELL { WHAT: EQTY(JOJO).AMOUNT(432).PRICE(29.01); }
}
'''