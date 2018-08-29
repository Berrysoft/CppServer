ALL:= obj/ bin/ bin/server.out bin/error.so bin/file.so bin/cpu.so bin/version.so bin/disk.so bin/markdown.so bin/modules bin/style.css 
.PHONY: all
all: $(ALL)
.PHONY: clean
clean:
	rm -rf obj/ bin/
.PHONY: run
run: $(ALL)
	cd bin/ && ./server.out
.PHONY: runv
runv: $(ALL)
	cd bin/ && ./server.out -v
obj/:
	mkdir obj
bin/:
	mkdir bin
obj/ioepoll.o: src/ioepoll.cpp src/ioepoll.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto 
obj/main.o: src/main.cpp src/server.h src/thread_pool.h src/http/http.h src/ioepoll.h src/safe_queue.h src/http/http_head.h src/http/http_request.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto 
obj/options.o: src/options.cpp src/options.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto 
obj/server.o: src/server.cpp src/server.h src/http/http_request.h src/thread_pool.h src/http/http.h src/ioepoll.h src/safe_queue.h src/http/http_head.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto 
obj/html_writer.o: src/html/html_writer.cpp src/html/html_writer.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto -fPIC
obj/http.o: src/http/http.cpp src/html/html_writer.h src/module/module.h src/module/response.h src/module/read_modules.h src/http/http.h src/http/http_head.h src/http/http_request.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto 
obj/http_head.o: src/http/http_head.cpp src/http/http_head.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto 
obj/http_request.o: src/http/http_request.cpp src/http/http_request.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto 
obj/http_url.o: src/http/http_url.cpp src/http/http_url.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto -fPIC
obj/module.o: src/module/module.cpp src/module/module.h src/http/http_request.h src/module/response.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto 
obj/read_modules.o: src/module/read_modules.cpp src/module/read_modules.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto -fPIC
obj/cpu.o: src/module/cpu/cpu.cpp src/html/html_writer.h src/module/cpu/cpu.h src/module/cpu/id.h src/module/cpu/proc_cpuinfo.h src/module/cpu/proc_stat.h src/module/response.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto -fPIC
obj/proc_cpuinfo.o: src/module/cpu/proc_cpuinfo.cpp src/module/cpu/proc_cpuinfo.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto -fPIC
obj/proc_stat.o: src/module/cpu/proc_stat.cpp src/module/cpu/proc_stat.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto -fPIC
obj/disk.o: src/module/disk/disk.cpp src/html/html_writer.h src/module/disk/disk.h src/module/response.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto -fPIC
obj/error.o: src/module/error/error.cpp src/module/error/error.h src/module/response.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto -fPIC
obj/file.o: src/module/file/file.cpp src/html/html_writer.h src/http/http_url.h src/module/file/file.h src/module/response.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto -fPIC
obj/markdown.o: src/module/markdown/markdown.cpp src/html/html_writer.h src/http/http_url.h src/module/markdown/markdown.h src/module/response.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto -fPIC
obj/version.o: src/module/version/version.cpp src/html/html_writer.h src/module/version/version.h src/module/version/mem.h src/module/response.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto -fPIC
obj/mem.o: src/module/version/mem.cpp src/module/version/mem.h 
	g++ -o $@ -c $< -std=c++17 -O2 -Wall -flto -fPIC
bin/server.out: obj/main.o obj/options.o obj/server.o obj/ioepoll.o obj/http.o obj/http_request.o obj/http_head.o obj/http_url.o obj/module.o obj/read_modules.o 
	g++ -o $@ $^ -lpthread -ldl -lstdc++fs -flto
bin/error.so: obj/error.o 
	g++ -o $@ $^ -shared -fPIC -flto
bin/file.so: obj/file.o obj/http_url.o obj/html_writer.o obj/read_modules.o 
	g++ -o $@ $^ -lstdc++fs -shared -fPIC -flto
bin/cpu.so: obj/cpu.o obj/proc_cpuinfo.o obj/proc_stat.o obj/html_writer.o 
	g++ -o $@ $^ -shared -fPIC -flto
bin/version.so: obj/version.o obj/mem.o obj/html_writer.o 
	g++ -o $@ $^ -shared -fPIC -flto
bin/disk.so: obj/disk.o obj/html_writer.o 
	g++ -o $@ $^ -shared -fPIC -flto
bin/markdown.so: obj/markdown.o obj/http_url.o obj/html_writer.o 
	g++ -o $@ $^ -lstdc++fs -shared -fPIC -flto
bin/modules: src/module/modules
	cp $^ $@
bin/style.css: src/html/style.css
	cp $^ $@
