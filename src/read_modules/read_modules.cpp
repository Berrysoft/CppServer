#include <fstream>
#include <linq/query.hpp>
#include <linq/string.hpp>
#include <linq/to_container.hpp>
#include <module/read_modules.h>

using namespace std;
using namespace linq;

const char modules_file[] = "modules";

vector<string> read_modules_file()
{
    ifstream ifs(modules_file);
    if (ifs.is_open())
    {
        return read_lines(ifs) >>
               select([](string line) {
                   size_t i = line.find_first_not_of(" \t\r");
                   if (i != string::npos)
                       return line.substr(i);
                   else
                       return string{};
               }) >>
               where([](string line) { return !line.empty() && (line.front() != '#'); }) >>
               to_vector<string>();
    }
    return {};
}
