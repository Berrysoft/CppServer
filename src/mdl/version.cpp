#include "version.h"
#include "html_writer.h"
#include "mem.h"
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdio>
#include <ctime>

using namespace std;

string get_version()
{
    ifstream ifs("/proc/version");
    string result;
    getline(ifs, result);
    return result;
}

const char *weekdays[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};

string get_time()
{
    time_t timep;
    tm *p;
    time(&timep);
    p = localtime(&timep);
    ostringstream oss;
    oss << (1900 + p->tm_year) << "年" << (1 + p->tm_mon) << "月" << p->tm_mday << "日 ";
    oss << weekdays[p->tm_wday] << ' ' << p->tm_hour << ':' << p->tm_min << ':' << p->tm_sec;
    return oss.str();
}

ssize_t version_response::send(int fd)
{
    ssize_t result = 0;
    html_writer writer(fd);
    result += writer.write_head("大作业-系统信息");
    result += writer.write_h1("<a href=\"..//proc/version\">系统版本</a>");
    result += writer.write_p(get_version());

    result += writer.write_h1("本地时间");
    result += writer.write_p(get_time());

    result += writer.write_h1("<a href=\"..//proc/meminfo\">内存信息</a>");
    mem_info info = read_proc_meminfo();
    vector<string> texts;
    texts.push_back("Total");
    texts.push_back("Free");
    texts.push_back("Buffers");
    texts.push_back("Cached");
    texts.push_back("Active");
    texts.push_back("Inactive");
    result += writer.write_table_start(texts);
    texts.clear();
    int *p = (int *)(&info);
    for (int i = 0; i < 6; i++)
    {
        char buffer[16];
        sprintf(buffer, "%d kB", p[i]);
        texts.push_back(buffer);
    }
    result += writer.write_tr(texts);
    result += writer.write_table_end();
    result += writer.write_end();
    return result;
}

void *get_instance_response(const char *command)
{
    return new version_response();
}
