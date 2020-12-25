#ifndef CPPNP_CURRENT_THREAD_H_
#define CPPNP_CURRENT_THREAD_H_
#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread{
    extern __thread pid_t t_cacheTid;
    inline pid_t tid(){
        if(t_cacheTid == 0)
            t_cacheTid = static_cast<pid_t>(::syscall(SYS_gettid));
        return t_cacheTid;
    }
}

#endif // CPPNP_CURRENT_THREAD_H_
