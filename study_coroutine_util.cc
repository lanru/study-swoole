#include "study_coroutine.h"
#include <unordered_map>

using Study::PHPCoroutine;
using Study::Coroutine;

ZEND_BEGIN_ARG_INFO_EX(arginfo_study_coroutine_create, 0, 0, 1)
                ZEND_ARG_CALLABLE_INFO(0, func, 0)
ZEND_END_ARG_INFO()

// 协程接口方法声明,采用php方法的写法
//static PHP_METHOD(study_coroutine_util, create);

// 从PHP_METHOD变成PHP_FUNCTION，从php方法变成了php函数,php方法采用ZEND_METHOD(classname, name)方式命名，php函数不用类名。
PHP_FUNCTION (study_coroutine_create) {
    //zend_fcall_info就是用来接收我们创建协程的时候传递的那个函数。我们使用
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    //ZEND_PARSE_PARAMETERS_START(1, -1)的-1代表没有限制传递给Study\Coroutine::create接口函数的最大参数个数限制，也就是可变参数
    ZEND_PARSE_PARAMETERS_START(1, -1)
            Z_PARAM_FUNC(fci, fcc)
            // Z_PARAM_VARIADIC这个宏是用来解析可变参数的，'*'对于Z_PARAM_VARIADIC实际上并没有用到。*表示可变参数可传或者不传递。与之对应的是'+'，表示可变参数至少传递一个
            Z_PARAM_VARIADIC('*', fci.params, fci.param_count)
            //ZEND_PARSE_PARAMETERS_END_EX的原因是因为我们希望在解析参数失败的时候，会向PHP返回一个false
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    //以下是直接调用函数的方法，未使用协程
    long cid = PHPCoroutine::create(&fcc, fci.param_count, fci.params);
    RETURN_LONG(cid);

}

static std::unordered_map<long, Coroutine *> user_yield_coros;

ZEND_BEGIN_ARG_INFO_EX(arginfo_study_coroutine_void, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_METHOD (study_coroutine_util, yield) {
    //获取当前协程*Study::Coroutine::current
    Coroutine *co = Coroutine::get_current();
    //把获取的当前协程存入一个无序字典user_yield_coros里面
    user_yield_coros[co->get_cid()] = co;
    co->yield();
    RETURN_TRUE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_study_coroutine_resume, 0, 0, 1)
                ZEND_ARG_INFO(0, cid)
ZEND_END_ARG_INFO()

PHP_METHOD (study_coroutine_util, resume) {
    zend_long cid = 0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_LONG(cid)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    auto coroutine_iterator = user_yield_coros.find(cid);
    if (coroutine_iterator == user_yield_coros.end()) {
        php_error_docref(NULL, E_WARNING, "resume error");
        RETURN_FALSE;
    }

    Coroutine *co = coroutine_iterator->second;
    user_yield_coros.erase(cid);
    co->resume();
    RETURN_TRUE;
}

PHP_METHOD (study_coroutine_util, getCid) {
    Coroutine *co = Coroutine::get_current();
    if (co == nullptr)
    {
        RETURN_LONG(-1);
    }
    RETURN_LONG(co->get_cid());
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_study_coroutine_isExist, 0, 0, 1)
                ZEND_ARG_INFO(0, cid)
ZEND_END_ARG_INFO()

PHP_METHOD (study_coroutine_util, isExist) {
    zend_long cid = 0;
    bool is_exist;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_LONG(cid)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    auto coroutine_iterator = Coroutine::coroutines.find(cid);
    is_exist = (coroutine_iterator != Coroutine::coroutines.end());
    RETURN_BOOL(is_exist);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_study_coroutine_defer, 0, 0, 1)
                ZEND_ARG_CALLABLE_INFO(0, func, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(study_coroutine_util, defer)
{
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
    php_study_fci_fcc *defer_fci_fcc;

    defer_fci_fcc = (php_study_fci_fcc *)emalloc(sizeof(php_study_fci_fcc));

    ZEND_PARSE_PARAMETERS_START(1, -1)
            Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    defer_fci_fcc->fci = fci;
    defer_fci_fcc->fcc = fcc;

    PHPCoroutine::defer(defer_fci_fcc);
}


/**
 * zend_function_entry
 * typedef struct _zend_function_entry {
    const char *fname;
    zif_handler handler;
    const struct _zend_internal_arg_info *arg_info;
    uint32_t num_args;
    uint32_t flags;
    } zend_function_entry;
 * handler是一个函数指针，也就是该函数的主体。那么是什么样的函数指针呢？我们得看看前面的zif_handler：typedef void (ZEND_FASTCALL *zif_handler)(INTERNAL_FUNCTION_PARAMETERS); 在这里，这个handler存放的就是函数指针zim_study_coroutine_util_create
 *
 */
const zend_function_entry study_coroutine_util_methods[] =
        {
//       原先php方法的写法,https://php-extension-research.github.io/study/#/协程基础模块/协程短名（二）中删除
//                PHP_ME(study_coroutine_util, create, arginfo_study_coroutine_create, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

//      https://php-extension-research.github.io/study/#/协程基础模块/协程短名（二）
                ZEND_FENTRY(create, ZEND_FN(study_coroutine_create), arginfo_study_coroutine_create, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
                PHP_ME(study_coroutine_util, yield, arginfo_study_coroutine_void, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
                PHP_ME(study_coroutine_util, resume, arginfo_study_coroutine_resume, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
                PHP_ME(study_coroutine_util, getCid, arginfo_study_coroutine_void, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
                PHP_ME(study_coroutine_util, isExist, arginfo_study_coroutine_isExist, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
                PHP_ME(study_coroutine_util, defer, arginfo_study_coroutine_defer, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
                PHP_FE_END
        };

zend_class_entry study_coroutine_ce;
zend_class_entry *study_coroutine_ce_ptr;

//    协程接口收集
void study_coroutine_util_init() {
    PHPCoroutine::init();
    INIT_NS_CLASS_ENTRY(study_coroutine_ce, "Study", "Coroutine", study_coroutine_util_methods);
    study_coroutine_ce_ptr = zend_register_internal_class(&study_coroutine_ce
                                                          TSRMLS_CC);
    zend_register_class_alias("SCo", study_coroutine_ce_ptr); // 新增的代码
    // Registered in the Zend Engine
}