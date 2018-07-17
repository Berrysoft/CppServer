# Linux中的C++大作业
本程序是课程《Linux中的C++》的大作业，采用Socket与Epoll技术，实现了HTTP1.1的部分功能。
## 使用说明
可以在目录下运行以下命令编译并启动服务器：
``` bash
make run
```
make命令需要保证`obj`与`bin`文件夹存在。

然后在浏览器中输入`http://127.0.0.1:3342/`访问。

也可以直接克隆Git仓库并编译运行：
``` bash
git clone https://github.com/Berrysoft/CppServer.git
cd CppServer
mkdir obj
mkdir bin
make run
```
## 程序文件
本程序需要以下文件才能相应HTTP请求：
* server

本程序需要以下文件才能正常使用：
* modules
* file.so（文件模块）

以下均为本程序的可选文件：
* cpu.so（CPU模块）
* version.so（系统模块）
* disk.so（硬盘模块）
* markdown.so（自述模块）
* style.css（HTML样式表）
* ../README.md（自述文件）

## 功能说明
在服务器端输入`r`刷新模块加载，输入`c`清理所有连接，输入`q`退出。

默认超过2分钟无响应的连接会自动清理。

默认启动4个线程，监听数为16384，Epoll的等待时间为2秒。

由于程序采用Chunked传输，并且会自动关闭产生错误或者客户端断开的Socket，在页面长按`F5`（即频繁刷新）可能使得页面暂时显示不正常，只需再次刷新即可。

默认提供了以下模块：
* 查看文件
* CPU与进程信息
* 系统版本、时间与内存信息
* 硬盘信息
* 说明文档（简单Markdown解释器）

### 查看文件
查看文件模块是必需的，如果删除会导致400错误。

系统信息也可以作为文件打开。
### CPU与进程信息
通过`/proc/cpuinfo`与`/proc/stat`获取的CPU信息。

通过系统函数获取的进程信息，并链接到`/proc`目录下的对应进程的`/status`。
### 系统版本、时间与内存信息
基本上是`/proc`中的文件或者系统函数。
### 硬盘信息
WSL没有`/proc/partitions`文件，因此采用VFS文件系统获取根目录信息。
### 说明文档
说明文档用的是`../README.md`绝对路径，打开的就是本文件。
## 为本程序开发模块
想要为本程序开发模块，需要引入`mdl`文件夹下的头文件`response.h`，并实现`void *get_instance_response(const char *command)`方法。这个方法应当返回一个指向继承`response`的类的指针，并可以被`delete`。

`response`类只定义了一个抽象方法`ssize_t send(int fd)`，这是用来向文件描述符`fd`直接写HTML文档的函数。建议引入`html_writer.h`头文件写HTML。

`html_writer.h`中定义了3个有用的宏：
* `INIT_RESULT_AND_TEMP`：初始化两个类型为`ssize_t`的变量，名为`result`和`t`。
* `IF_NEGATIVE_EXIT(exp)`：判断`exp`是否为负值，如果为负，返回；反之加在`result`上。
* `RETURN_RESULT`：返回`result`。
