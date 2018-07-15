# 主程序
server.out: main.o server.o sem.o html_content.o module.o read_modules.o
	g++ -o server.out main.o server.o sem.o html_content.o module.o read_modules.o -lpthread -ldl
main.o: main.cpp server.h thread_pool.h sem.h module.h html_content.h response.h
	g++ -c main.cpp -std=c++11
server.o: server.cpp server.h thread_pool.h sem.h html_content.h module.h read_modules.h
	g++ -c server.cpp -std=c++11
sem.o: sem.cpp sem.h
	g++ -c sem.cpp -std=c++11
html_content.o: html_content.cpp html_content.h response.h module.h
	g++ -c html_content.cpp -std=c++11
module.o: module.cpp module.h
	g++ -c module.cpp -std=c++11

# HTML Writer 目标对象
html_writer.o: html_writer.cpp html_writer.h
	g++ -c html_writer.cpp -std=c++11 -fPIC

# Read module 目标对象
read_modules.o: read_modules.cpp read_modules.h
	g++ -c read_modules.cpp -std=c++11 -fPIC

# file模块
file.so: file.o html_writer.o read_modules.o
	g++ -shared -o file.so file.o html_writer.o read_modules.o
file.o: file.cpp response.h html_writer.h read_modules.h
	g++ -c file.cpp -std=c++11 -fPIC

# cpu模块
cpu.so: cpu.o proc_cpuinfo.o proc_stat.o html_writer.o
	g++ -shared -o cpu.so cpu.o proc_cpuinfo.o proc_stat.o html_writer.o
cpu.o: cpu.cpp cpu.h response.h id.h proc_cpuinfo.h proc_stat.h html_writer.h
	g++ -c cpu.cpp -std=c++11 -fPIC
proc_cpuinfo.o: proc_cpuinfo.cpp proc_cpuinfo.h
	g++ -c proc_cpuinfo.cpp -std=c++11 -fPIC
proc_stat.o: proc_stat.cpp proc_stat.h
	g++ -c proc_stat.cpp -std=c++11 -fPIC

# version模块
version.so: version.o mem.o html_writer.o
	g++ -shared -o version.so version.o mem.o html_writer.o
version.o: version.cpp version.h response.h mem.h html_writer.h
	g++ -c version.cpp -std=c++11 -fPIC
mem.o: mem.cpp mem.h
	g++ -c mem.cpp -std=c++11 -fPIC

# disk模块
disk.so: disk.o html_writer.o
	g++ -shared -o disk.so disk.o html_writer.o
disk.o: disk.cpp disk.h html_writer.h
	g++ -c disk.cpp -std=c++11 -fPIC

# Markdown模块
markdown.so: markdown.o html_writer.o
	g++ -shared -o markdown.so markdown.o html_writer.o
markdown.o: markdown.cpp markdown.h html_writer.h
	g++ -c markdown.cpp -std=c++11 -fPIC

# 运行
run:
	./server.out

# 清理
clean:
	rm *.o
