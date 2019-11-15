#pragma once

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdint.h>
#include <sys/types.h>

    struct init_response_arg
    {
        const char* method;
        const char* module;
        const char* command;
        const char* args;
        const char* content;
        double version;
    };

    int32_t res_init(struct init_response_arg* arg);
    int32_t res_status();
    int64_t res_length();
    const char* res_type();
    ssize_t res_send(int fd);
    int32_t res_destory();

#ifdef __cplusplus
}
#endif // __cplusplus
