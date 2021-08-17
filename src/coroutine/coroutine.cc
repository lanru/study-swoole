#include "../../include/coroutine.h"

using study::Coroutine;

Coroutine *Coroutine::current = nullptr;
long Coroutine::last_cid = 0;
std::unordered_map<long, Coroutine *> Coroutine::coroutines;
size_t Coroutine::stack_size = DEFAULT_C_STACK_SIZE;
st_coro_on_swap_t Coroutine::on_yield = nullptr;
st_coro_on_swap_t Coroutine::on_resume = nullptr;
st_coro_on_swap_t Coroutine::on_close = nullptr;

void *Coroutine::get_current_task() {
    return current ? current->get_task() : nullptr;
}

void *Coroutine::get_task() {
    return task;
}

Coroutine *Coroutine::get_current() {
    return current;
}

void Coroutine::set_task(void *_task) {
    task = _task;
}

long Coroutine::create(coroutine_func_t fn, void *args) {
    return (new Coroutine(fn, args))->run();
}

void Coroutine::yield() {
    assert(current == this);
    on_yield(task);
    current = origin;
    ctx.swap_out();
}

void Coroutine::resume() {
    assert(current != this);
    on_resume(task);
    origin = current;
    current = this;
    ctx.swap_in();
    if (ctx.is_end()) {
        assert(current == this);
        on_close(task);
        current = origin;
        coroutines.erase(cid);
        delete this;
    }
}

void Coroutine::set_on_yield(st_coro_on_swap_t func) {
    on_yield = func;
}

void Coroutine::set_on_resume(st_coro_on_swap_t func) {
    on_resume = func;
}

void Coroutine::set_on_close(st_coro_on_swap_t func) {
    on_close = func;
}

static void sleep_timeout(uv_timer_t *timer) {
    //这个函数的作用就是去resume当前协程，于是就起到了唤醒当前协程的效果
    ((Coroutine *) timer->data)->resume();
}

int Coroutine::sleep(double seconds) {
    Coroutine *co = Coroutine::get_current();

    uv_timer_t timer;
    timer.data = co;
    //用来初始化我们的定时器节点uv_timer_init的位置是mac或者linxu中的/usr/local/include/uv.h
    uv_timer_init(uv_default_loop(), &timer);
    //用来把这个定时器节点插入到整个定时器堆里面。这里，我们需要明确一点，libuv的这个定时器堆是一个最小堆，也就是说，堆顶的定时器节点的timeout越小。uv_timer_start函数里面会把定时器节点timer插入到整个定时器堆里面，并且会根据timer的timeout调整timer在定时器堆里面的位置
    uv_timer_start(&timer, sleep_timeout, seconds * 1000, 0);
    //用来切换出当前协程，模拟出了协程自身阻塞的效果
    co->yield();
    return 0;
}
