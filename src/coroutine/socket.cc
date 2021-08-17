//
// Created by wlh on 2021/8/17.
//

#include "coroutine_socket.h"
#include "coroutine.h" // 增加的一行
#include "socket.h"

using study::coroutine::Socket;
using study::Coroutine;

Socket::Socket(int domain, int type, int protocol) {
    sockfd = stSocket_create(domain, type, protocol);
    if (sockfd < 0) {
        return;
    }
    stSocket_set_nonblock(sockfd);
}

int Socket::bind(int type, char *host, int port) {
    return stSocket_bind(sockfd, type, host, port);
}

int Socket::listen() {
    return stSocket_listen(sockfd);
}

int Socket::accept() {
    int connfd;

    connfd = stSocket_accept(sockfd);
    // 尝试获取客户端连接，如果返回值connfd小于0，并且errno == EAGAIN,那么就说明此时没有客户端连接，我们就需要等待事件（此时的事件是有客户端的连接，这是一个可读的事件）的到来，并且yield这个协程。我们把这个等待和yield的操作封装在了函数wait_event里面
    if (connfd < 0 && errno == EAGAIN) {
        wait_event(ST_EVENT_READ);
        connfd = stSocket_accept(sockfd);
    }

    return connfd;
}

bool Socket::wait_event(int event) {
    long id;
    Coroutine *co;
    epoll_event *ev;

    co = Coroutine::get_current();
    id = co->get_cid();

    // 用来判断这个协程需要等待那种类型的事件，目前是支持READ和WRITE
    if (!StudyG.poll) {
        init_stPoll();
    }
    ev = StudyG.poll->events;

    ev->events = event == ST_EVENT_READ ? EPOLLIN : EPOLLOUT;
    ev->data.u64 = touint64(sockfd, id);
    // 用来把事件注册到全局的epollfd上面
    epoll_ctl(StudyG.poll->epollfd, EPOLL_CTL_ADD, sockfd, ev);

    co->yield();
    return true;
}

ssize_t Socket::recv(void *buf, size_t len) {
    int ret;

    ret = stSocket_recv(sockfd, buf, len, 0);
    if (ret < 0 && errno == EAGAIN) {
        wait_event(ST_EVENT_READ);
        ret = stSocket_recv(sockfd, buf, len, 0);
    }
    return ret;
}

ssize_t Socket::send(const void *buf, size_t len) {
    int ret;

    ret = stSocket_send(sockfd, buf, len, 0);
    if (ret < 0 && errno == EAGAIN) {
        wait_event(ST_EVENT_WRITE);
        ret = stSocket_send(sockfd, buf, len, 0);
    }
    return ret;
}

Socket::~Socket() {
}




