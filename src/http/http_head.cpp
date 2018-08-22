#include "http_head.h"
#include <sys/socket.h>

const char head[] = "HTTP/1.1 200 OK\r\nServer: Berrysoft.Linux.Cpp.Server\r\n\r\n";

ssize_t http_head::send(int fd)
{
    return ::send(fd, head, sizeof(head) - 1, 0);
}
