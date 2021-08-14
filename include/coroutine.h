#ifndef COROUTINE_H
#define COROUTINE_H
#define DEFAULT_C_STACK_SIZE (2 * 1024 * 1024)
#include "context.h"
#include <unordered_map>

namespace Study {
    class Coroutine {
    protected:
        long cid;
        static Coroutine *current;
        void *task = nullptr;
        Coroutine *origin;
        static size_t stack_size;
        static long last_cid;
        Context ctx;
        // fn就是我们的PHPCoroutine::create_func，它完成了我们创建协程的基础工作，例如创建PHP栈帧、把传递给用户函数的参数放到栈帧上面、执行用户空间传递过来的函数（实际上就是去执行zend_op_array）。函数PHPCoroutine::create_func最终会被我们的协程入口函数调用（协程入口函数我们后面会去实现它）。
        //  private_data是需要给协程跑的一些参数，实际上是fn在使用private_data。
        //  然后，再传递这两个值去构造ctx，这个是创建出的这个协程的上下文，可以说是协程库最核心的地方了（这个上下文我们后面会讲）。其中，stack_size是协程栈的默认大小，我们定义在src/coroutine/coroutine.cc里面初始化的
        Coroutine(coroutine_func_t fn, void *private_data) :
                ctx(stack_size, fn, private_data) {
            // cid是用来记录创建出的这个协程的id。我们在Study::Coroutine这个类里面进行声明
            cid = ++last_cid;
            coroutines[cid] = this;
        }

        long run() {
            long cid = this->cid;
            printf("in Coroutine run method: co[%ld] start\n", cid);
            // 因为我们需要跑一个新的协程，所以当前正在跑的协程是origin的，这个刚被创建的协程才是current，是现在需要被执行的协程
            origin = current;
            current = this;
            // 这个函数的作用和ctx.swap_out刚好相反。ctx.swap_in()的作用是加载上下文ctx。所以，当我们执行run函数的时候，这个协程的上下文ctx就会被加载，所以这个最底层的协程就会被运行，所以我们的PHP协程就会被运行
            ctx.swap_in();
            if (ctx.is_end())
            {
                cid = current->get_cid();
                printf("in run method: co[%ld] end\n", cid);
                current = origin;
                coroutines.erase(cid);
                delete this;
            }
            return cid;
        }

    public:
        static long create(coroutine_func_t fn, void *args = nullptr);

        static void *get_current_task();
        static Coroutine *get_current();
        static std::unordered_map<long, Coroutine *> coroutines;
        inline long get_cid()
        {
            return cid;
        }
        void *get_task();
        void set_task(void *_task);
        void yield();
        void resume();
    };
}

#endif    /* COROUTINE_H */
