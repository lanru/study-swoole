#ifndef STUDY_H_
#define STUDY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// include standard library
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <queue>

typedef struct {
    int epollfd; // 用来保存我们创建的那个epollfd
    int ncap;
    int event_num; // 新增加的一行
    struct epoll_event *events; //是用来保存epoll返回的事件
} stPoll_t;

typedef struct {
    int running; // 新增加的一行
    stPoll_t *poll;
} stGlobal_t;
//为什么我们不直接在文件study.h里面定义全局变量StudyG呢？因为，如果我们在文件study.h里面定义了这个全局的变量，那么，如果头文件study.h被多个地方引入了，那么，编译器就会认为这个全局变量重复定义了，所以，我们需要在一个地方去定义它，然后在另一个地方声明这是一个外部变量
extern stGlobal_t StudyG;

enum stEvent_type {
    ST_EVENT_NULL = 0,
    ST_EVENT_DEAULT = 1u << 8,
    ST_EVENT_READ = 1u << 9,
    ST_EVENT_WRITE = 1u << 10,
    ST_EVENT_RDWR = ST_EVENT_READ | ST_EVENT_WRITE,
    ST_EVENT_ERROR = 1u << 11,
};

static inline uint64_t touint64(int fd, int id) {
    uint64_t ret = 0;
    ret |= ((uint64_t) fd) << 32;
    ret |= ((uint64_t) id);

    return ret;
}


static inline void fromuint64(uint64_t v, int *fd, int *id) {
    *fd = (int) (v >> 32);
    *id = (int) (v & 0xffffffff);
}

int init_stPoll();

int free_stPoll();

int st_event_init();

int st_event_wait();

int st_event_free();


#endif /* STUDY_H_ */
