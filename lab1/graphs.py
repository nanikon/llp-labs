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

f = open("ram_data", "r")
data = f.read().strip().split("\n")
a, b = [], []
	
for i in data:
	x, y = i.split(", ")
	a.append(int(x))
	b.append(float(y))
	
plt.plot(a, b)
plt.xlabel("Время с началанагрузочных тестов, с")
plt.ylabel("Занимаемый процессом объем RAM, KB")
plt.title("Нагрузочное тестирование")
plt.show() 
f.close()

f = open("file_size", "r")
data = f.read().strip().split("\n")
a, b = [], []
	
for i in data:
	x, y = i.split(",")
	a.append(int(x))
	b.append(float(y))
	
plt.plot(a, b)
plt.xlabel("Количество нод в дереве, шт")
plt.ylabel("Размер файла, байт")
plt.title("Занимаемое место на диске")
plt.show() 
f.close()