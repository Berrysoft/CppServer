#include <fstream>
#include <html/html_writer.h>
#include <linq/query.hpp>
#include <linq/string.hpp>
#include <linq/to_container.hpp>
#include <module/disk/disk.h>
#include <sf/sformat.hpp>
#include <string>
#include <sys/statfs.h>
#include <vector>

using namespace std;
using namespace sf;
using namespace linq;

ssize_t disk_response::send(int fd)
{
    INIT_RESULT_AND_TEMP;
    html_writer writer(fd);
    IF_NEGATIVE_EXIT(writer.write_head("大作业-硬盘"));
    IF_NEGATIVE_EXIT(writer.write_h1("硬盘信息"));
    vector<string> texts{ "扇区大小", "总扇区数", "总大小", "剩余扇区数", "剩余大小" };
    IF_NEGATIVE_EXIT(writer.write_table_start(texts));
    struct statfs disk;
    statfs("/", &disk);
    unsigned long long b = disk.f_bsize;
    unsigned long long bs = disk.f_blocks;
    unsigned long long bss = bs * b;
    unsigned long long fbs = disk.f_bfree;
    unsigned long long fbss = fbs * b;
    texts.clear();
    texts.push_back(to_string(b) + " B");
    texts.push_back(to_string(bs));
    texts.push_back(sprint("{:f2} GB", bss / 1073741824.0));
    texts.push_back(to_string(fbs));
    texts.push_back(sprint("{:f2} GB", fbss / 1073741824.0));
    IF_NEGATIVE_EXIT(writer.write_tr(texts));
    IF_NEGATIVE_EXIT(writer.write_table_end());
    IF_NEGATIVE_EXIT(writer.write_h1("<a href=\"../file//proc/partitions\">硬盘分区信息</a>"));
    texts.assign({ "名称", "大小" });
    IF_NEGATIVE_EXIT(writer.write_table_start(texts));
    {
        ifstream ifs{ "/proc/partitions" };
        for (const string& line : read_lines(ifs) >> skip(2))
        {
            vector<string_view> arr = line >> split(' ') >> where([](auto s) { return !s.empty(); }) >> to_vector<string_view>();
            texts.assign({ string(arr[3]), sprint("{:f2} GB", stoull(string(arr[2])) / 1048576.0) });
            IF_NEGATIVE_EXIT(writer.write_tr(texts));
        }
    }
    IF_NEGATIVE_EXIT(writer.write_table_end());
    IF_NEGATIVE_EXIT(writer.write_end());
    RETURN_RESULT;
}

void* get_instance_response(void* request)
{
    const http_request& req = *(const http_request*)request;
    if (req.version() > 1.0)
    {
        return new disk_response(req);
    }
    return nullptr;
}

void delete_instance_response(void* response)
{
    disk_response* res = (disk_response*)response;
    delete res;
}
