import matplotlib.pyplot as plt
	
f = open("insert_time", "r")
data = f.read().strip().split("\n")
a, b = [], []
	
for i in data:
	x, y = i.split(",")
	a.append(int(x))
	b.append(float(y))
	
plt.plot(a, b)
plt.xlabel("Количество нод в дереве, шт")
plt.ylabel("Время, с")
plt.title("Вставка")
plt.show() 
f.close()

f = open("search_time", "r")
data = f.read().strip().split("\n")
a, b = [], []
	
for i in data:
	x, y = i.split(",")
	a.append(int(x))
	b.append(float(y))
	
plt.plot(a, b)
plt.xlabel("Количество нод в дереве, шт")
plt.ylabel("Время, с")
plt.title("Поиск по условию")
plt.show() 
f.close()


f = open("update_time", "r")
data = f.read().strip().split("\n")
a, b = [], []
	
for i in data:
	x, y = i.split(",")
	a.append(int(x))
	b.append(float(y))
	
plt.plot(a, b)
plt.xlabel("Количество нод в дереве, шт")
plt.ylabel("Время, с")
plt.title("Изменение ноды")
plt.show() 
f.close()

f = open("delete_time", "r")
data = f.read().strip().split("\n")
a, b = [], []
	
for i in data:
	x, y = i.split(",")
	a.append(int(x))
	b.append(float(y))
	
plt.plot(a, b)
plt.xlabel("Количество нод в дереве, шт")
plt.ylabel("Время, с")
plt.title("Удаление ноды (или поддерева)")
plt.show() 
f.close()