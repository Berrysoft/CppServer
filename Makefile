SOURCE := src/
MODULE := src/mdl/
BIN := bin/
OBJ := obj/

STD := -std=c++11
STDFIPC := -std=c++11 -fPIC

# 编译所有
.PHONY: all
ALL := $(BIN)server.out $(BIN)file.so $(BIN)cpu.so $(BIN)version.so $(BIN)disk.so $(BIN)markdown.so $(BIN)modules $(BIN)style.css
all: $(ALL)

.PHONY: run
run: $(ALL)
	cd $(BIN) && ./server.out

# 主程序
$(BIN)server.out: $(OBJ)main.o $(OBJ)server.o $(OBJ)html_content.o $(OBJ)module.o $(OBJ)read_modules.o
	g++ -o $(BIN)server.out $^ -lpthread -ldl
$(OBJ)main.o: $(SOURCE)main.cpp $(SOURCE)server.h $(SOURCE)thread_pool.h $(SOURCE)module.h $(SOURCE)html_content.h $(MODULE)response.h
	g++ -o $(OBJ)main.o -c $(SOURCE)main.cpp $(STD)
$(OBJ)server.o: $(SOURCE)server.cpp $(SOURCE)server.h $(SOURCE)thread_pool.h $(SOURCE)apply_tuple.h $(SOURCE)html_content.h $(SOURCE)module.h $(SOURCE)read_modules.h
	g++ -o $(OBJ)server.o -c $(SOURCE)server.cpp $(STD)
$(OBJ)html_content.o: $(SOURCE)html_content.cpp $(SOURCE)html_content.h $(MODULE)response.h $(SOURCE)module.h
	g++ -o $(OBJ)html_content.o -c $(SOURCE)html_content.cpp $(STD)
$(OBJ)module.o: $(SOURCE)module.cpp $(SOURCE)module.h
	g++ -o $(OBJ)module.o -c $(SOURCE)module.cpp $(STD)

# HTML Writer 目标对象
$(OBJ)html_writer.o:$(MODULE)html_writer.cpp $(MODULE)html_writer.h
	g++ -o $(OBJ)html_writer.o -c $(MODULE)html_writer.cpp $(STDFIPC)

# Read module 目标对象
$(OBJ)read_modules.o: $(SOURCE)read_modules.cpp $(SOURCE)read_modules.h
	g++ -o $(OBJ)read_modules.o -c $(SOURCE)read_modules.cpp $(STDFIPC)

# file模块
$(BIN)file.so: $(OBJ)file.o $(OBJ)html_writer.o $(OBJ)read_modules.o
	g++ -shared -o $(BIN)file.so $^
$(OBJ)file.o: $(MODULE)file.cpp $(MODULE)response.h $(MODULE)html_writer.h $(SOURCE)read_modules.h
	g++ -o $(OBJ)file.o -c $(MODULE)file.cpp $(STDFIPC)

# cpu模块
$(BIN)cpu.so: $(OBJ)cpu.o $(OBJ)proc_cpuinfo.o $(OBJ)proc_stat.o $(OBJ)html_writer.o
	g++ -shared -o $(BIN)cpu.so $^
$(OBJ)cpu.o: $(MODULE)cpu.cpp $(MODULE)cpu.h $(MODULE)response.h $(MODULE)id.h $(MODULE)proc_cpuinfo.h $(MODULE)proc_stat.h $(MODULE)html_writer.h
	g++ -o $(OBJ)cpu.o -c $(MODULE)cpu.cpp $(STDFIPC)
$(OBJ)proc_cpuinfo.o: $(MODULE)proc_cpuinfo.cpp $(MODULE)proc_cpuinfo.h
	g++ -o $(OBJ)proc_cpuinfo.o -c $(MODULE)proc_cpuinfo.cpp $(STDFIPC)
$(OBJ)proc_stat.o: $(MODULE)proc_stat.cpp $(MODULE)proc_stat.h
	g++ -o $(OBJ)proc_stat.o -c $(MODULE)proc_stat.cpp $(STDFIPC)

# version模块
$(BIN)version.so: $(OBJ)version.o $(OBJ)mem.o $(OBJ)html_writer.o
	g++ -shared -o $(BIN)version.so $^
$(OBJ)version.o: $(MODULE)version.cpp $(MODULE)version.h $(MODULE)response.h $(MODULE)mem.h $(MODULE)html_writer.h
	g++ -o $(OBJ)version.o -c $(MODULE)version.cpp $(STDFIPC)
$(OBJ)mem.o: $(MODULE)mem.cpp $(MODULE)mem.h
	g++ -o $(OBJ)mem.o -c $(MODULE)mem.cpp $(STDFIPC)

# disk模块
$(BIN)disk.so: $(OBJ)disk.o $(OBJ)html_writer.o
	g++ -shared -o $(BIN)disk.so $^
$(OBJ)disk.o: $(MODULE)disk.cpp $(MODULE)disk.h $(MODULE)html_writer.h
	g++ -o $(OBJ)disk.o -c $(MODULE)disk.cpp $(STDFIPC)

# Markdown模块
$(BIN)markdown.so: $(OBJ)markdown.o $(OBJ)html_writer.o
	g++ -shared -o $(BIN)markdown.so $^
$(OBJ)markdown.o: $(MODULE)markdown.cpp $(MODULE)markdown.h $(MODULE)html_writer.h
	g++ -o $(OBJ)markdown.o -c $(MODULE)markdown.cpp $(STDFIPC)

# 特殊文件
$(BIN)modules: $(SOURCE)modules
	cp $(SOURCE)modules $(BIN)modules

$(BIN)style.css: $(SOURCE)style.css
	cp $(SOURCE)style.css $(BIN)style.css

# 清理
.PHONY: clean
clean:
	rm $(OBJ)/*.o
