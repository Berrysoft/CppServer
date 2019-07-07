#include <ctime>
#include <fstream>
#include <html/html_writer.h>
#include <module/version/mem.h>
#include <module/version/version.h>
#include <sf/sformat.hpp>
#include <string>
#include <vector>

using namespace std;
using namespace sf;

string get_version()
{
    ifstream ifs("/proc/version");
    string result;
    getline(ifs, result);
    return result;
}

const char* const weekdays[] = { "星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六" };

string get_time()
{
    time_t timep = time(nullptr);
    tm* p = localtime(&timep);
    return sprint("{0}年{1}月{2}日 {3} {4}:{5}:{6}", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, weekdays[p->tm_wday], p->tm_hour, p->tm_min, p->tm_sec);
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
    int* p = (int*)(&info);
    for (int i = 0; i < 6; i++)
    {
        texts.push_back(to_string(p[i]) + " kB");
    }
    IF_NEGATIVE_EXIT(writer.write_tr(texts));
    IF_NEGATIVE_EXIT(writer.write_table_end());
    IF_NEGATIVE_EXIT(writer.write_end());
    RETURN_RESULT;
}

void* get_instance_response(void* request)
{
    const http_request& req = *(const http_request*)request;
    if (req.version() > 1.0)
    {
        return new version_response(req);
    }
    return nullptr;
}

void delete_instance_response(void* response)
{
    version_response* res = (version_response*)response;
    delete res;
}
