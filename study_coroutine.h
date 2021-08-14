#include "php_study.h"
#include "coroutine.h"

#define DEFAULT_PHP_STACK_PAGE_SIZE       8192
#define PHP_CORO_TASK_SLOT ((int)((ZEND_MM_ALIGNED_SIZE(sizeof(php_coro_task)) + ZEND_MM_ALIGNED_SIZE(sizeof(zval)) - 1) / ZEND_MM_ALIGNED_SIZE(sizeof(zval))))
#include <stack>
struct php_coro_args {
    zend_fcall_info_cache *fci_cache;
    zval *argv;
    uint32_t argc;
};
struct php_study_fci_fcc
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
};
// save the coroutine stack info
struct php_coro_task {
    zval *vm_stack_top; // 协程栈栈顶。
    zval *vm_stack_end; // 协程栈栈底
    zend_vm_stack vm_stack; // 协程栈指针
    size_t vm_stack_page_size;//协程栈页大小
    zend_execute_data *execute_data; // 当前协程栈的栈帧
    Study::Coroutine *co;
    std::stack<php_study_fci_fcc *> *defer_tasks;
};

namespace Study {
    class PHPCoroutine {
    public:
        static long create(zend_fcall_info_cache *fci_cache, uint32_t argc, zval *argv);
        static void defer(php_study_fci_fcc *defer_fci_fcc);

    protected:
        static void save_task(php_coro_task *task);

        static void save_vm_stack(php_coro_task *task);

        static php_coro_task *get_task();

        static php_coro_task main_task;

        static void create_func(void *arg);

        static void vm_stack_init(void);
    };
}
