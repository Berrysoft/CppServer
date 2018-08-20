//获取进程ID。
#pragma once
#include <unistd.h>

struct id
{
    uid_t uid;
    uid_t euid;
    gid_t gid;
    gid_t egid;
    pid_t pid;
};

id get_id()
{
    return {getuid(), geteuid(), getgid(), getegid(), getpid()};
}