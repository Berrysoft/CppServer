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

bool version_response::supports(const char* version)
{
    string v(version);
    return v != "HTTP/1.0";
}

ssize_t version_response::send(int fd)
{
    INIT_RESULT_AND_TEMP;
    html_writer writer(fd);
    IF_NEGATIVE_EXIT(writer.write_head("大作业-系统信息"));
    IF_NEGATIVE_EXIT(writer.write_h1("<a href=\"../file//proc/version\">系统版本</a>"));
    IF_NEGATIVE_EXIT(writer.write_p(get_version()));

    IF_NEGATIVE_EXIT(writer.write_h1("本地时间"));
    IF_NEGATIVE_EXIT(writer.write_p(get_time()));

    IF_NEGATIVE_EXIT(writer.write_h1("<a href=\"../file//proc/meminfo\">内存信息</a>"));
    mem_info info = read_proc_meminfo();
    vector<string> texts;
    texts.push_back("Total");
    texts.push_back("Free");
    texts.push_back("Buffers");
    texts.push_back("Cached");
    texts.push_back("Active");
    texts.push_back("Inactive");
    IF_NEGATIVE_EXIT(writer.write_table_start(texts));
    texts.clear();
    int *p = (int *)(&info);
    for (int i = 0; i < 6; i++)
    {
        char buffer[16];
        sprintf(buffer, "%d kB", p[i]);
        texts.push_back(buffer);
    }
    IF_NEGATIVE_EXIT(writer.write_tr(texts));
    IF_NEGATIVE_EXIT(writer.write_table_end());
    IF_NEGATIVE_EXIT(writer.write_end());
    RETURN_RESULT;
}

void *get_instance_response(const char *command)
{
    return new version_response();
}
