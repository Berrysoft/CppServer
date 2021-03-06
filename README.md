# Linux中的C++大作业
本程序是课程《Linux中的C++》的大作业，采用Socket与Epoll，实现了HTTP1.1的部分功能。
## 使用说明
克隆Git仓库并编译运行：
``` bash
$ git clone https://github.com/Berrysoft/CppServer.git
$ cd CppServer
$ mkdir build
$ cd build
$ cmake .. -GNinja
$ ninja
$ ./server
```
编译本程序需要我开发的库[StreamFormat](https://github.com/Berrysoft/StreamFormat)与[CppLinq](https://github.com/Berrysoft/CppLinq)。

如果需要复杂的参数：
``` bash
$ ./server -v -a 127.0.0.1 -p 8080 -c 64 -t 16 -e 1000 -i 1 -o 2
```
希望查看提示可以使用`-h`或者`--help`命令：
```
$ ./server -h
Berrysoft.Linux.Cpp.Server
网站在Edge、IE、Chrome、FireFox、Safari(iPhone)下测试通过。
请使用支持chunked的浏览器打开网站。
用法：./server [-h] [-p port] [-c count] [-t threads]
                [epoll-timeout] [clock-interval] [clock-timeout]
选项：
-h --help               获取帮助
-v --verbose            显示详细信息
-a --address            设置监听地址，默认为所有IP
-p --port               设置监听端口，默认为3342
-c --count              设置监听数，默认为16384
-t --threads            设置线程数，默认为4
-e --epoll-timeout      设置Epoll的等待时间，默认为2000(ms)
-i --clock-interval     设置时钟间隔，默认为60(s)
-o --clock-timeout      设置时钟等待循环数，默认为2（个）
```
## 程序文件
本程序需要以下文件才能相应HTTP请求：
* `server`

本程序需要以下文件才能正常使用：
* `modules.txt`（模块列表文件）
* `libread_modules.so`（读取列表模块）
* `libhtml_writer.so`（HTML模块）
* `libfile.so`（文件模块）

以下均为本程序的可选文件：
* `libcpu.so`（CPU模块）
* `libversion.so`（系统模块）
* `libdisk.so`（硬盘模块）
* `libmarkdown.so`（Markdown解释模块）
* `style.css`（HTML样式表）
* `README.md`（自述文件）
## 功能
在服务器端输入`r`刷新模块加载，输入`c`清理所有连接，输入`q`退出。

可以通过更改`modules.txt`文件实现动态更改模块加载。

超过时钟等待循环数的连接会自动清理。一些浏览器（如Edge）会一直使用一个连接进行导航，可能因此导致连接中断，刷新即可。

由于程序采用Chunked传输（除错误网页），并且会自动关闭产生错误或者客户端断开的Socket，在页面长按`F5`（即频繁刷新）可能使得页面暂时显示不正常，只需再次刷新即可。

默认提供了以下模块：
* 查看文件
* CPU与进程信息
* 系统版本、时间与内存信息
* 硬盘信息
* Markdown解释器
### 查看文件
查看文件模块是必需的，如果删除会导致400错误。

可以在GET或POST时附加数据`raw=<整数>`，其中整数值为非零时，返回请求文件的源内容。
### CPU与进程信息
通过`/proc/cpuinfo`与`/proc/stat`获取的CPU信息。

通过系统函数获取的进程信息，并链接到`/proc`目录下的对应进程的`/status`。
### 系统版本、时间与内存信息
基本上是`/proc`中的文件或者系统函数。
### 硬盘信息
WSL没有`/proc/partitions`文件，因此采用VFS文件系统获取根目录信息。
### Markdown解释器
默认用的是`README.md`绝对路径，打开的就是本文件。

这个解释器使用了`md4c-html`库来实现转换。
## 压力测试
下面是用ApacheBench进行并发100、总数1000000的压力测试结果：
```
$ ab -k -n 1000000 -c 100 http://127.0.0.1:3342/

...

Server Software:        Berrysoft.Linux.Cpp.Server
Server Hostname:        127.0.0.1
Server Port:            3342

Document Path:          /
Document Length:        0 bytes

Concurrency Level:      100
Time taken for tests:   12.473 seconds
Complete requests:      1000000
Failed requests:        0
Non-2xx responses:      1000000
Keep-Alive requests:    1000000
Total transferred:      130000000 bytes
HTML transferred:       0 bytes
Requests per second:    80172.08 [#/sec] (mean)
Time per request:       1.247 [ms] (mean)
Time per request:       0.012 [ms] (mean, across all concurrent requests)
Transfer rate:          10178.10 [Kbytes/sec] received

...
```
## 为本程序开发模块
想要为本程序开发模块，需要引入`include/module/`文件夹下的头文件`response.h`，并实现相应的方法。其中`int32_t res_init(struct init_response_arg* arg)`与`ssize_t res_send(int fd)`必须实现。前者根据请求初始化响应，后者向文件描述符写入响应内容。

由于是C语言接口，理论上任何一个可以写C接口动态库的语言（如Rust或C#）都可以为本程序开发模块。

如果使用C/C++，建议引入`html_writer.h`头文件写HTML，这里面定义了一个向文件描述符写HTML文档的类，并使用了`style.css`的绝对路径。

`html_writer.h`中还定义了3个有用的宏：
``` cpp
#define INIT_RESULT_AND_TEMP ssize_t result = 0, t

#define IF_NEGATIVE_EXIT(exp) \
    if ((t = (exp)) < 0)      \
        return t;             \
    result += t

#define RETURN_RESULT return result
```

如果处理`send`或者`write`等函数的返回值不当，可能会阻塞线程，导致服务器无响应。
## 许可证
本程序源代码采用MIT许可证授权。
