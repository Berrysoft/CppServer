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
## 功能说明
在服务器端输入`r`刷新模块加载，输入`c`清理所有连接，输入`q`退出。

默认超过2分钟无响应的连接会自动清理。

默认启动4个线程，监听数为16384，Epoll的等待时间为2秒。

由于程序会关闭产生错误或者客户端断开的Socket，在页面长按`F5`（即频繁刷新）可能使得页面暂时显示不正常，只需再次刷新即可。

拖慢这个服务器的最关键因素是屏幕输出，如果因为频繁访问导致服务器不正常，请注释掉所有`printf`语句。

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
