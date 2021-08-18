//
// Created by wlh on 2021/8/17.
//

#include "coroutine_socket.h"
#include "coroutine.h" // 增加的一行
#include "socket.h"
#include "log.h"

using study::coroutine::Socket;
using study::Coroutine;

char *Socket::read_buffer = nullptr; // 增加的地方
size_t Socket::read_buffer_len = 0; // 增加的地方
char *Socket::write_buffer = nullptr; // 增加的地方
size_t Socket::write_buffer_len = 0; // 增加的地方

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

    do {
        connfd = stSocket_accept(sockfd);
    } while (connfd < 0 && errno == EAGAIN && wait_event(ST_EVENT_READ));

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
    (StudyG.poll->event_num)++; // 新增的一行
    co->yield();
    // 以下是增加的代码
    if (epoll_ctl(StudyG.poll->epollfd, EPOLL_CTL_DEL, sockfd, NULL) < 0) {
        stWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
        return false;
    }
    return true;
}

ssize_t Socket::recv(void *buf, size_t len) {
    int ret;

    do {
        ret = stSocket_recv(sockfd, buf, len, 0);
    } while (ret < 0 && errno == EAGAIN && wait_event(ST_EVENT_READ));

    return ret;
}

ssize_t Socket::send(const void *buf, size_t len) {
    int ret;

    do {
        ret = stSocket_send(sockfd, buf, len, 0);
    } while (ret < 0 && errno == EAGAIN && wait_event(ST_EVENT_WRITE));

    return ret;
}

Socket::Socket(int fd) {
    sockfd = fd;
    stSocket_set_nonblock(sockfd);
}

Socket::~Socket() {
}

int Socket::init_read_buffer() {
    if (!read_buffer) {
        try {
            read_buffer = new char[65536];
        }
        catch (const std::bad_alloc &e) {
            stError("%s", e.what());
        }

        read_buffer_len = 65536;
    }

    return 0;
}


int Socket::init_write_buffer() {
    if (!write_buffer) {
        try {
            write_buffer = new char[65536];
        }
        catch (const std::bad_alloc &e) {
            stError("%s", e.what());
        }

        write_buffer_len = 65536;
    }

    return 0;
}


int Socket::close() {
    return stSocket_close(sockfd);
}

