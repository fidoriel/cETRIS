ary = [[' ', '@', ' '],
       ['@', '@', '@']]

h = 2
w = 3

aryn = []
for i in range(w):
    tmp = []
    for j in reversed(range(h)):
        tmp.append(ary[j][i])
    aryn.append(tmp)
print(aryn)

ary2 = []
for i in reversed(range(w)):
    tmp = []
    for j in range(h):
        tmp.append(ary[j][i])
    ary2.append(tmp)
print(ary2)