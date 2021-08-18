//
// Created by wlh on 2021/8/17.
//

#include "study.h"
#include "coroutine.h"
#include "log.h"
#include "timer.h"

using study::Coroutine;
using study::Timer;
using study::TimerManager;
using study::timer_manager;

TimerManager study::timer_manager;

const uint64_t Timer::MILLI_SECOND = 1;
const uint64_t Timer::SECOND = 1000;

stGlobal_t StudyG;

int init_stPoll() {
    try {
        StudyG.poll = new stPoll_t();
    }
    catch (const std::exception &e) {
        stError("%s", e.what());
    }

    StudyG.poll->epollfd = epoll_create(256);
    if (StudyG.poll->epollfd < 0) {
        stWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
        delete StudyG.poll;
        StudyG.poll = nullptr;
        return -1;
    }

    StudyG.poll->ncap = 16;
    try {
        StudyG.poll->events = new epoll_event[StudyG.poll->ncap](); // zero initialized
    }
    catch (const std::bad_alloc &e) {
        stError("%s", e.what());
    }
    StudyG.poll->event_num = 0;

    return 0;
}


int free_stPoll() {
    if (close(StudyG.poll->epollfd) < 0) {
        stWarn("Error has occurred: (errno %d) %s", errno, strerror(errno));
    }
    delete[] StudyG.poll->events;
    StudyG.poll->events = nullptr;
    delete StudyG.poll;
    StudyG.poll = nullptr;
    return 0;
}


int st_event_init() {
    if (!StudyG.poll) {
        init_stPoll();
    }
    StudyG.running = 1; // 新增的一行
    return 0;
}


int st_event_wait() {
    st_event_init();

    while (StudyG.running > 0) {
        int n;
        uint64_t timeout;
        epoll_event *events;

        timeout = timer_manager.get_next_timeout();
        events = StudyG.poll->events;
        n = epoll_wait(StudyG.poll->epollfd, events, StudyG.poll->ncap, timeout);

        for (int i = 0; i < n; i++) {
            int fd;
            int id;
            struct epoll_event *p = &events[i];
            uint64_t u64 = p->data.u64;
            Coroutine *co;

            fromuint64(u64, &fd, &id);
            co = Coroutine::get_by_cid(id);
            co->resume();
        }

        timer_manager.run_timers();

        if (timer_manager.get_next_timeout() < 0 && StudyG.poll->event_num == 0) {
            StudyG.running = 0;
        }
    }

    st_event_free();

    return 0;
}

int st_event_free() {
    StudyG.running = 0; // 增加的代码
    free_stPoll();
    return 0;
}

uint64_t Timer::get_current_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

Timer::Timer(uint64_t _timeout, timer_func_t _callback, void *_private_data, TimerManager *_timer_manager) :
        timeout(_timeout), callback(_callback), private_data(_private_data), timer_manager(_timer_manager) {
    exec_msec = get_current_ms() + _timeout;
}

TimerManager::TimerManager() {
}

TimerManager::~TimerManager() {
}

void TimerManager::add_timer(int64_t _timeout, timer_func_t _callback, void *_private_data) {
    Timer *timer = new Timer(_timeout, _callback, _private_data, this);
    timers.push(timer);
}

int64_t TimerManager::get_next_timeout() {
    int64_t diff;

    if (timers.empty()) {
        return -1;
    }
    Timer *t = timers.top();

    diff = t->exec_msec - Timer::get_current_ms();
    return diff < 0 ? 0 : diff;
}

void TimerManager::run_timers() {
    uint64_t now = Timer::get_current_ms();
    while (true) {
        if (timers.empty()) {
            break;
        }
        Timer *t = timers.top();
        if (now < t->exec_msec) {
            break;
        }
        timers.pop();
        t->callback(t->private_data);
        delete t;
    }
}


