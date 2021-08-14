//#include "coroutine.h"
#include "../../include/coroutine.h"


using Study::Coroutine;

Coroutine *Coroutine::current = nullptr;
long Coroutine::last_cid = 0;
std::unordered_map<long, Coroutine *> Coroutine::coroutines;
size_t Coroutine::stack_size = DEFAULT_C_STACK_SIZE;


void *Coroutine::get_current_task() {
    return Coroutine::current ? Coroutine::current->get_task() : nullptr;
}

long Coroutine::create(coroutine_func_t fn, void* args)
{
    // 首先，我们创建了一个协程对象new Coroutine(fn, args)
    return (new Coroutine(fn, args))->run();
}

void *Coroutine::get_task() {
    return task;
}

void Coroutine::set_task(void *_task)
{
    task = _task;
}

Coroutine *Coroutine::get_current()
{
    return current;
}

void Coroutine::yield()
{
    current = origin;
    ctx.swap_out();
}

void Coroutine::resume()
{
    origin = current;
    current = this;
    ctx.swap_in();
    if (ctx.is_end())
    {
        cid = current->get_cid();
        printf("in resume method: co[%ld] end\n", cid);
        current = origin;
        coroutines.erase(cid);
        delete this;
    }
}
