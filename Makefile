.PHONY: all
ALL:= bin/server.out bin/error.so bin/file.so bin/cpu.so bin/version.so bin/disk.so bin/markdown.so bin/modules bin/style.css 
all: $(ALL)
.PHONY: run
run: $(ALL)
	cd bin/ && ./server.out
.PHONY: runv
runv: $(ALL)
	cd bin/ && ./server.out -v
.PHONY: clean
clean: 
	rm obj//* bin//*
obj/server.o: src/server.cpp src/server.h src/http/http_request.h src/thread_pool.h src/http/http.h src/safe_queue.h src/http/http_response.h 
	g++ -o $@ -c src/server.cpp -std=c++17 -O2 -Wall -flto 
obj/main.o: src/main.cpp src/server.h src/thread_pool.h src/http/http.h src/safe_queue.h src/http/http_response.h src/http/http_request.h 
	g++ -o $@ -c src/main.cpp -std=c++17 -O2 -Wall -flto 
obj/html_writer.o: src/html/html_writer.cpp src/html/html_writer.h 
	g++ -o $@ -c src/html/html_writer.cpp -std=c++17 -O2 -Wall -flto -fPIC
obj/http.o: src/http/http.cpp src/module/read_modules.h src/http/http.h src/http/http_get.h src/http/http_head.h src/http/http_response.h src/http/http_request.h 
	g++ -o $@ -c src/http/http.cpp -std=c++17 -O2 -Wall -flto 
obj/http_get.o: src/http/http_get.cpp src/module/module.h src/module/response.h src/http/http_get.h src/http/http_response.h src/http/http_request.h 
	g++ -o $@ -c src/http/http_get.cpp -std=c++17 -O2 -Wall -flto 
obj/http_head.o: src/http/http_head.cpp src/http/http_head.h src/http/http_response.h 
	g++ -o $@ -c src/http/http_head.cpp -std=c++17 -O2 -Wall -flto 
obj/http_request.o: src/http/http_request.cpp src/http/http_request.h 
	g++ -o $@ -c src/http/http_request.cpp -std=c++17 -O2 -Wall -flto 
obj/module.o: src/module/module.cpp src/module/module.h src/module/response.h 
	g++ -o $@ -c src/module/module.cpp -std=c++17 -O2 -Wall -flto 
obj/read_modules.o: src/module/read_modules.cpp src/module/read_modules.h 
	g++ -o $@ -c src/module/read_modules.cpp -std=c++17 -O2 -Wall -flto -fPIC
obj/cpu.o: src/module/cpu/cpu.cpp src/html/html_writer.h src/module/cpu/cpu.h src/module/cpu/id.h src/module/cpu/proc_cpuinfo.h src/module/cpu/proc_stat.h src/module/response.h 
	g++ -o $@ -c src/module/cpu/cpu.cpp -std=c++17 -O2 -Wall -flto -fPIC
obj/proc_cpuinfo.o: src/module/cpu/proc_cpuinfo.cpp src/module/cpu/proc_cpuinfo.h 
	g++ -o $@ -c src/module/cpu/proc_cpuinfo.cpp -std=c++17 -O2 -Wall -flto -fPIC
obj/proc_stat.o: src/module/cpu/proc_stat.cpp src/module/cpu/proc_stat.h 
	g++ -o $@ -c src/module/cpu/proc_stat.cpp -std=c++17 -O2 -Wall -flto -fPIC
obj/disk.o: src/module/disk/disk.cpp src/html/html_writer.h src/module/disk/disk.h src/module/response.h 
	g++ -o $@ -c src/module/disk/disk.cpp -std=c++17 -O2 -Wall -flto -fPIC
obj/error.o: src/module/error/error.cpp src/module/error/error.h src/module/response.h 
	g++ -o $@ -c src/module/error/error.cpp -std=c++17 -O2 -Wall -flto -fPIC
obj/file.o: src/module/file/file.cpp src/html/html_writer.h src/module/file/file.h src/module/response.h 
	g++ -o $@ -c src/module/file/file.cpp -std=c++17 -O2 -Wall -flto -fPIC
obj/markdown.o: src/module/markdown/markdown.cpp src/html/html_writer.h src/module/markdown/markdown.h src/module/response.h 
	g++ -o $@ -c src/module/markdown/markdown.cpp -std=c++17 -O2 -Wall -flto -fPIC
obj/version.o: src/module/version/version.cpp src/html/html_writer.h src/module/version/version.h src/module/version/mem.h src/module/response.h 
	g++ -o $@ -c src/module/version/version.cpp -std=c++17 -O2 -Wall -flto -fPIC
obj/mem.o: src/module/version/mem.cpp src/module/version/mem.h 
	g++ -o $@ -c src/module/version/mem.cpp -std=c++17 -O2 -Wall -flto -fPIC
bin/server.out: obj/main.o obj/server.o obj/http.o obj/http_request.o obj/http_get.o obj/http_head.o obj/module.o obj/read_modules.o 
	g++ -o $@ $^ -lpthread -ldl -lstdc++fs -flto
bin/error.so: obj/error.o 
	g++ -o $@ $^ -shared -fPIC -flto
bin/file.so: obj/file.o obj/html_writer.o obj/read_modules.o 
	g++ -o $@ $^ -lstdc++fs -shared -fPIC -flto
bin/cpu.so: obj/cpu.o obj/proc_cpuinfo.o obj/proc_stat.o obj/html_writer.o 
	g++ -o $@ $^ -shared -fPIC -flto
bin/version.so: obj/version.o obj/mem.o obj/html_writer.o 
	g++ -o $@ $^ -shared -fPIC -flto
bin/disk.so: obj/disk.o obj/html_writer.o 
	g++ -o $@ $^ -shared -fPIC -flto
bin/markdown.so: obj/markdown.o obj/html_writer.o 
	g++ -o $@ $^ -lstdc++fs -shared -fPIC -flto
bin/modules: src/module/modules
	cp $^ $@
bin/style.css: src/html/style.css
	cp $^ $@
