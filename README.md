# Linux中的C++大作业
## 使用说明
默认已经编译好程序。可以在目录下运行以下命令编译并启动服务器：
```
make run
```
make命令需要保证`obj`与`bin`文件夹存在。

然后在浏览器中输入`http://127.0.0.1:3342/`访问。
## 功能说明
在服务器端输入`r`刷新模块加载，输入`q`退出。

默认提供了以下模块：
* 查看文件
* CPU与进程信息
* 系统版本、时间与内存信息
* 硬盘信息
* 说明文档（简单Markdown解释器）

查看文件模块是必需的，如果删除会导致400错误。

说明文档用的是“../README.md”绝对路径。

其实系统信息也可以作为文件打开。
## 头文件说明
这个文件夹下的头文件太多了，在这里列一个说明：
### server.h
服务器头文件，管理一个循环线程和一个线程池。
### thread_pool.h
线程池模板类。
### sem.h
信号量。线程池用的大部分是C++11封装的类，但是由于C++11没有信号量，只好自己用pthread写一个。
### response.h
在动态模块中用来向文件描述符写HTML的抽象类。
### module.h
模块类，用于动态加载模块。
### read_modules.h
一个方法，用来读取`modules`文件。
### html_content.h
响应HTTP请求的类。类名有“html”是由于之前在这里直接回复HTML，后来分离了功能。
### html_writer.h
用来向一个文件描述符写HTML文档，需要用到程序运行目录下的`style.css`样式表。
### file.h
有一个`file_response`类，继承`response`，用来读取并输出文件内容，或者动态生成主页。
### cpu.h
有一个`cpu_response`类，继承`response`，用来输出CPU、时间与进程信息。
### proc_cpuinfo.h
生成CPU信息。
### proc_stat.h
生成CPU使用信息。
### id.h
获取进程ID。
### version.h
有一个`version_response`类，继承`response`，用来输出系统版本和内存信息。
### mem.h
生成内存信息。
### disk.h
生成硬盘信息。由于开发环境WSL没有/proc/partitions文件，只能用statfs获取根目录信息（实际上是C盘的信息）。
### markdown.h
一个简易Markdown引擎，为`README.md`文件生成HTML。
## 程序文件
本程序需要以下文件才能相应HTTP请求：
* server

本程序需要以下文件才能正常使用：
* modules
* file.so

以下均为本程序的可选文件：
* cpu.so
* version.so
* disk.so
* markdown.so
* style.css
* README.md
