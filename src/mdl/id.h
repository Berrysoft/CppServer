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
    id result = {getuid(), geteuid(), getgid(), getegid(), getpid()};
    return result;
}