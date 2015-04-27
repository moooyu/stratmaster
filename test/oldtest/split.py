f = open('testcase','r')
line = f.readline()
case = 7
while line:
    with open("test/case"+str(case),'w') as g:
        line = f.readline()
        while line and line[:2] != '//':
            g.write(line)
            line = f.readline()
    case += 1

