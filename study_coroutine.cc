#include "study_coroutine.h"
#include "./include/coroutine.h"

using Study::Coroutine;
using Study::PHPCoroutine;
using namespace std;

php_coro_task PHPCoroutine::main_task = {0};

void PHPCoroutine::init()
{
    Coroutine::set_on_yield(on_yield);
    Coroutine::set_on_resume(on_resume);
    Coroutine::set_on_close(on_close);
}

long PHPCoroutine::create(zend_fcall_info_cache *fci_cache, uint32_t argc, zval *argv)
{
    php_coro_args php_coro_args;
    php_coro_args.fci_cache = fci_cache;
    php_coro_args.argv = argv;
    php_coro_args.argc = argc;
    save_task(get_task());
    return Coroutine::create(create_func, (void *)&php_coro_args);
}

void PHPCoroutine::save_task(php_coro_task *task)
{
    save_vm_stack(task);
}

void PHPCoroutine::save_vm_stack(php_coro_task *task)
{
    task->vm_stack_top = EG(vm_stack_top);
    task->vm_stack_end = EG(vm_stack_end);
    task->vm_stack = EG(vm_stack);
    task->vm_stack_page_size = EG(vm_stack_page_size);
    task->execute_data = EG(current_execute_data);
}

php_coro_task *PHPCoroutine::get_task()
{
    //我们让Coroutine::get_current_task()返回一个void类型的指针，然后，根据我们上层需要的协程结构进行转换即可，这样我们的协程库就可以在多个地方使用了。所以，我们需要去实现Study::Coroutine::get_current_task
    php_coro_task *task = (php_coro_task *)Coroutine::get_current_task();
    return task ? task : &main_task;
}

void PHPCoroutine::create_func(void *arg)
{
    // 这一段代码只是简单的把一些核心内容提取出来，存放在其他变量里面。这一段代码纯粹是为了增强代码的可读性
    int i;
    php_coro_args *php_arg = (php_coro_args *)arg;
    zend_fcall_info_cache fci_cache = *php_arg->fci_cache;
    zend_function *func = fci_cache.function_handler;
    zval *argv = php_arg->argv;
    int argc = php_arg->argc;
    php_coro_task *task;
    zend_execute_data *call;
    zval _retval, *retval = &_retval;
    // 这个方法的目的是初始化一个新的PHP栈，因为我们即将要创建一个协程了
    vm_stack_init();
    call = (zend_execute_data *)(EG(vm_stack_top));
    task = (php_coro_task *)EG(vm_stack_top);
    zval *top = (zval *)((char *)call + PHP_CORO_TASK_SLOT * sizeof(zval));
    EG(vm_stack_top) = top;

    // 这段代码的作用是分配一个zend_execute_data，分配一块用于当前作用域的内存空间。因为用户空间的函数会被编译成zend_op_array，zend_op_array是在zend_execute_data上通过opline来执行的。而我们上面的fci_cache.function_handler是一个zend_function，这个zend_function是一个union，里面包含了一些函数的公共信息以及具体的函数类型，用户定义的函数或者内部函数（内部函数指的是PHP内置的函数或者C扩展提供的函数）
#if PHP_VERSION_ID < 70400
    call = zend_vm_stack_push_call_frame(
        ZEND_CALL_TOP_FUNCTION | ZEND_CALL_ALLOCATED, func, argc, fci_cache.called_scope, fci_cache.object);
#else
    do
    {
        uint32_t call_info;
        void *object_or_called_scope;
        if ((func->common.fn_flags & ZEND_ACC_STATIC) || !fci_cache.object)
        {
            object_or_called_scope = fci_cache.called_scope;
            call_info = ZEND_CALL_TOP_FUNCTION | ZEND_CALL_DYNAMIC;
        }
        else
        {
            object_or_called_scope = fci_cache.object;
            call_info = ZEND_CALL_TOP_FUNCTION | ZEND_CALL_DYNAMIC | ZEND_CALL_HAS_THIS;
        }
        call = zend_vm_stack_push_call_frame(call_info, func, argc, object_or_called_scope);
    } while (0);
#endif
    // 这段代码的作用是用来逐个获取zend_execute_data上第i个参数应该在的zval地址，然后把地址值赋值给param，获取到了参数地址之后，我们就可以把我们传递给用户函数的参数一个一个的拷贝到zend_execute_data上面去
    for (i = 0; i < argc; ++i)
    {
        zval *param;
        zval *arg = &argv[i];
        param = ZEND_CALL_ARG(call, i + 1);
        ZVAL_COPY(param, arg);
    }

    call->symbol_table = nullptr;
    // 初始化call这个zend_execute_data之后，我们把它赋值给EG(current_execute_data)
    EG(current_execute_data) = call;
    EG(error_handling) = EH_NORMAL;
    EG(exception_class) = nullptr;
    EG(exception) = nullptr;
    save_vm_stack(task);

    task->co = Coroutine::get_current();
    task->co->set_task((void *)task);
    task->defer_tasks = nullptr;
    // 把当前的协程栈信息保存在task里面
    if (func->type == ZEND_USER_FUNCTION)
    {
        ZVAL_UNDEF(retval);
        EG(current_execute_data) = nullptr;
        // zend_init_func_execute_data的作用是去初始化zend_execute_data
        // * 会初始化zend_execute_data的以下字段：
        // * 1、zend_execute_data.opline，实际上就是zend_op_array.opcodes
        // * 2、zend_execute_data.call
        // * 3、zend_execute_data.return_value
        //
        //* 会设置executor_globals的以下字段：
        // * 1、executor_globals.current_execute_data
        zend_init_func_execute_data(call, &func->op_array, retval);
        // zend_execute_ex的作用就是去循环执行executor_globals.current_execute_data指向的opline。此时，这些opline就是我们用户空间传递的函数
        zend_execute_ex(EG(current_execute_data));
    }
    else /* ZEND_INTERNAL_FUNCTION */
    {
        ZVAL_NULL(retval);
        call->prev_execute_data = nullptr;
        call->return_value = nullptr; /* this is not a constructor call */
        execute_internal(call, retval);
        zend_vm_stack_free_args(call);
    }
    task = get_task();
    std::stack<php_study_fci_fcc *> *defer_tasks = task->defer_tasks;
    if (defer_tasks)
    {
        php_study_fci_fcc *defer_fci_fcc;
        zval result;
        while (!defer_tasks->empty())
        {
            defer_fci_fcc = defer_tasks->top();
            defer_tasks->pop();
            defer_fci_fcc->fci.retval = &result;

            if (zend_call_function(&defer_fci_fcc->fci, &defer_fci_fcc->fcc) != SUCCESS)
            {
                php_error_docref(NULL, E_WARNING, "defer execute error");
                return;
            }
            efree(defer_fci_fcc);
        }
        delete defer_tasks;
        task->defer_tasks = nullptr;
    }

    zval_ptr_dtor(retval);
}

void PHPCoroutine::vm_stack_init(void)
{
    // 首先，我们把我们定义好的默认PHP栈一页的大小赋值给size，我们在文件study_coroutine.h里面来进行声明：
    uint32_t size = DEFAULT_PHP_STACK_PAGE_SIZE;
    // 作用是从堆上面分配出size的大小的空间，然后把地址赋值给zend_vm_stack
    zend_vm_stack page = (zend_vm_stack)emalloc(size);
    // 这段代码的作用是把我们的堆模拟成栈的行为。因为按照cpp的内存模型，栈的地址空间一般是由高地址往低地址增长的
    // page->top的作用是指向目前的栈顶，这个top会随着栈里面的数据而不断的变化。压栈，top往靠近end的方向移动个；出栈，top往远离end的方向移动
    page->top = ZEND_VM_STACK_ELEMENTS(page);
    // page->end的作用就是用来标识PHP栈的边界，以防'栈溢出'。这个page->end可以作为是否要扩展PHP栈的依据
    page->end = (zval *)((char *)page + size);
    page->prev = NULL;
    // 这段代码的作用是去修改现在的PHP栈，让它指向我们申请出来的新的PHP栈空间
    EG(vm_stack) = page;
    EG(vm_stack)->top++;
    EG(vm_stack_top) = EG(vm_stack)->top;
    EG(vm_stack_end) = EG(vm_stack)->end;
    EG(vm_stack_page_size) = size;
}

void PHPCoroutine::defer(php_study_fci_fcc *defer_fci_fcc)
{
    php_coro_task *task = (php_coro_task *)get_task();
    if (task->defer_tasks == nullptr)
    {
        task->defer_tasks = new std::stack<php_study_fci_fcc *>;
    }
    task->defer_tasks->push(defer_fci_fcc);
}

void PHPCoroutine::on_yield(void *arg)
{
    php_coro_task *task = (php_coro_task *)arg;
    php_coro_task *origin_task = get_origin_task(task);
    save_task(task);
    restore_task(origin_task);
}

void PHPCoroutine::on_resume(void *arg)
{
    php_coro_task *task = (php_coro_task *)arg;
    php_coro_task *current_task = get_task();
    save_task(current_task);
    restore_task(task);
}

// todo  恢复php的执行栈，此处非常重要，否则会报错:Assertion failed: ((executor_globals.vm_stack_top) > (zval *) (executor_globals.vm_stack) && (executor_globals.vm_stack_end) > (zval *) (executor_globals.vm_stack) && (executor_globals.vm_stack_top) <= (executor_globals.vm_stack_end)), function zend_vm_stack_free_call_frame_ex, file Zend/zend_execute.h, line 289.
void PHPCoroutine::on_close(void *arg)
{
    php_coro_task *task = (php_coro_task *)arg;
    php_coro_task *origin_task = get_origin_task(task);
    zend_vm_stack stack = EG(vm_stack);
    efree(stack);
    restore_task(origin_task);
}

/**
 * load PHP stack
 */
void PHPCoroutine::restore_task(php_coro_task *task)
{
    restore_vm_stack(task);
}

/**
 * load PHP stack
 */
inline void PHPCoroutine::restore_vm_stack(php_coro_task *task)
{
    EG(vm_stack_top) = task->vm_stack_top;
    EG(vm_stack_end) = task->vm_stack_end;
    EG(vm_stack) = task->vm_stack;
    EG(vm_stack_page_size) = task->vm_stack_page_size;
    EG(current_execute_data) = task->execute_data;
}
