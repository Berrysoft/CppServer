SOURCE := src/
MODULE := src/mdl/
BIN := bin/
OBJ := obj/

LTO := -flto
STD := -std=c++17 -O2 -Wall $(LTO)
STDFIPC := $(STD) -fPIC

# 编译所有
.PHONY: all
ALL := $(BIN)server.out $(BIN)error.so $(BIN)file.so $(BIN)cpu.so $(BIN)version.so $(BIN)disk.so $(BIN)markdown.so $(BIN)modules $(BIN)style.css
all: $(ALL)

.PHONY: run
run: $(ALL)
	cd $(BIN) && ./server.out

.PHONY: runv
runv: $(ALL)
	cd $(BIN) && ./server.out -v

# 主程序
$(BIN)server.out: $(OBJ)main.o $(OBJ)server.o $(OBJ)http.o $(OBJ)http_request.o $(OBJ)http_get.o $(OBJ)http_head.o $(OBJ)module.o $(OBJ)read_modules.o
	g++ -o $@ $^ -lpthread -ldl -lstdc++fs $(LTO)
$(OBJ)main.o: $(SOURCE)main.cpp $(SOURCE)server.h $(SOURCE)thread_pool.h $(SOURCE)http.h $(MODULE)response.h
	g++ -o $@ -c $(SOURCE)main.cpp $(STD)
$(OBJ)server.o: $(SOURCE)server.cpp $(SOURCE)server.h $(SOURCE)thread_pool.h $(SOURCE)http.h $(SOURCE)read_modules.h $(SOURCE)http_request.h
	g++ -o $@ -c $(SOURCE)server.cpp $(STD)
$(OBJ)http.o: $(SOURCE)http.cpp $(SOURCE)http.h $(SOURCE)read_modules.h $(SOURCE)http_request.h $(SOURCE)http_get.h $(SOURCE)http_head.h
	g++ -o $@ -c $(SOURCE)http.cpp $(STD)
$(OBJ)http_request.o: $(SOURCE)http_request.cpp $(SOURCE)http_request.h
	g++ -o $@ -c $(SOURCE)http_request.cpp $(STD)
$(OBJ)http_get.o: $(SOURCE)http_get.cpp $(SOURCE)http_get.h $(SOURCE)http_response.h $(MODULE)response.h $(SOURCE)http_request.h
	g++ -o $@ -c $(SOURCE)http_get.cpp $(STD)
$(OBJ)http_head.o: $(SOURCE)http_head.cpp $(SOURCE)http_head.h $(SOURCE)http_response.h
	g++ -o $@ -c $(SOURCE)http_head.cpp $(STD)
$(OBJ)module.o: $(SOURCE)module.cpp $(SOURCE)module.h
	g++ -o $@ -c $(SOURCE)module.cpp $(STD)

# HTML Writer 目标对象
$(OBJ)html_writer.o:$(MODULE)html_writer.cpp $(MODULE)html_writer.h
	g++ -o $@ -c $(MODULE)html_writer.cpp $(STDFIPC)

# Read module 目标对象
$(OBJ)read_modules.o: $(SOURCE)read_modules.cpp $(SOURCE)read_modules.h
	g++ -o $@ -c $(SOURCE)read_modules.cpp $(STDFIPC)

# error模块
$(BIN)error.so: $(OBJ)error.o
	g++ -shared -o $@ $^ $(LTO)
$(OBJ)error.o: $(MODULE)error.cpp $(MODULE)response.h
	g++ -o $@ -c $(MODULE)error.cpp $(STDFIPC)

# file模块
$(BIN)file.so: $(OBJ)file.o $(OBJ)html_writer.o $(OBJ)read_modules.o
	g++ -shared -o $@ $^ -lstdc++fs $(LTO)
$(OBJ)file.o: $(MODULE)file.cpp $(MODULE)response.h $(MODULE)html_writer.h $(SOURCE)read_modules.h
	g++ -o $@ -c $(MODULE)file.cpp $(STDFIPC)

# cpu模块
$(BIN)cpu.so: $(OBJ)cpu.o $(OBJ)proc_cpuinfo.o $(OBJ)proc_stat.o $(OBJ)html_writer.o
	g++ -shared -o $@ $^ $(LTO)
$(OBJ)cpu.o: $(MODULE)cpu.cpp $(MODULE)cpu.h $(MODULE)response.h $(MODULE)id.h $(MODULE)proc_cpuinfo.h $(MODULE)proc_stat.h $(MODULE)html_writer.h
	g++ -o $@ -c $(MODULE)cpu.cpp $(STDFIPC)
$(OBJ)proc_cpuinfo.o: $(MODULE)proc_cpuinfo.cpp $(MODULE)proc_cpuinfo.h
	g++ -o $@ -c $(MODULE)proc_cpuinfo.cpp $(STDFIPC)
$(OBJ)proc_stat.o: $(MODULE)proc_stat.cpp $(MODULE)proc_stat.h
	g++ -o $@ -c $(MODULE)proc_stat.cpp $(STDFIPC)

# version模块
$(BIN)version.so: $(OBJ)version.o $(OBJ)mem.o $(OBJ)html_writer.o
	g++ -shared -o $@ $^ $(LTO)
$(OBJ)version.o: $(MODULE)version.cpp $(MODULE)version.h $(MODULE)response.h $(MODULE)mem.h $(MODULE)html_writer.h
	g++ -o $@ -c $(MODULE)version.cpp $(STDFIPC)
$(OBJ)mem.o: $(MODULE)mem.cpp $(MODULE)mem.h
	g++ -o $@ -c $(MODULE)mem.cpp $(STDFIPC)

# disk模块
$(BIN)disk.so: $(OBJ)disk.o $(OBJ)html_writer.o
	g++ -shared -o $@ $^ $(LTO)
$(OBJ)disk.o: $(MODULE)disk.cpp $(MODULE)disk.h $(MODULE)html_writer.h
	g++ -o $@ -c $(MODULE)disk.cpp $(STDFIPC)

# Markdown模块
$(BIN)markdown.so: $(OBJ)markdown.o $(OBJ)html_writer.o
	g++ -shared -o $@ $^ -lstdc++fs $(LTO)
$(OBJ)markdown.o: $(MODULE)markdown.cpp $(MODULE)markdown.h $(MODULE)html_writer.h
	g++ -o $@ -c $(MODULE)markdown.cpp $(STDFIPC)

# 特殊文件
$(BIN)modules: $(MODULE)modules
	cp $^ $@

$(BIN)style.css: $(MODULE)style.css
	cp $^ $@

# 清理目标对象
.PHONY: clean
clean:
	rm $(OBJ)/*.*

# 清理全部文件
.PHONY: cleanall
cleanall:
	rm $(OBJ)/* $(BIN)/*
