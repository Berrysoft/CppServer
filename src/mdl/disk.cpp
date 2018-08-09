#include "disk.h"
#include "html_writer.h"
#include <vector>
#include <string>
#include <sys/statfs.h>
#include <cstdio>

using namespace std;

ssize_t disk_response::send(int fd)
{
    INIT_RESULT_AND_TEMP;
    html_writer writer(fd);
    IF_NEGATIVE_EXIT(writer.write_head("大作业-硬盘"));
    IF_NEGATIVE_EXIT(writer.write_h1("硬盘信息"));
    IF_NEGATIVE_EXIT(writer.write_p("由于WSL中没有<code>/proc/partitions</code>文件，本模块采用VFS文件系统获取根目录信息。"));
    vector<string> texts;
    texts.push_back("扇区大小");
    texts.push_back("总扇区数");
    texts.push_back("总大小");
    texts.push_back("剩余扇区数");
    texts.push_back("剩余大小");
    IF_NEGATIVE_EXIT(writer.write_table_start(texts));
    struct statfs disk;
    statfs("/", &disk);
    unsigned long long b = disk.f_bsize;
    unsigned long long bs = disk.f_blocks;
    unsigned long long bss = bs * b;
    unsigned long long fbs = disk.f_bfree;
    unsigned long long fbss = fbs * b;
    char buffer[16];
    texts.clear();
    sprintf(buffer, "%llu B", b);
    texts.push_back(buffer);
    sprintf(buffer, "%llu", bs);
    texts.push_back(buffer);
    sprintf(buffer, "%llu GB", bss >> 30);
    texts.push_back(buffer);
    sprintf(buffer, "%llu", fbs);
    texts.push_back(buffer);
    sprintf(buffer, "%llu GB", fbss >> 30);
    texts.push_back(buffer);
    IF_NEGATIVE_EXIT(writer.write_tr(texts));
    IF_NEGATIVE_EXIT(writer.write_table_end());
    IF_NEGATIVE_EXIT(writer.write_end());
    RETURN_RESULT;
}

void *get_instance_response(const char *command)
{
    return new disk_response();
}
