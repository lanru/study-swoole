#include "php_study.h"
// 新增加的内容
#include <stdio.h>
#include <iostream>
#include <uv.h>

using namespace std;

uint64_t repeat = 0;

static void callback(uv_timer_t *handle) {
    repeat = repeat + 1;
    cout << "repeat count:" << repeat << endl;
}

PHP_FUNCTION (study_timer_test) {
    uv_loop_t *loop = uv_default_loop();
    uv_timer_t timer_req;
    uv_timer_init(loop, &timer_req);

    uv_timer_start(&timer_req, callback, 1000, 1000);
    uv_run(loop, UV_RUN_DEFAULT);
}
// 新增加的内容
PHP_MINIT_FUNCTION (study) {
    // 协程类注册
    study_coroutine_util_init();
    // server注册
    study_coroutine_server_coro_init(); // 新增加的代码
    // chanel注册
    study_coro_channel_init();
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION (study) {
    return SUCCESS;
}

PHP_RINIT_FUNCTION (study) {
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION (study) {
    return SUCCESS;
}

PHP_MINFO_FUNCTION (study) {
    php_info_print_table_start();
    php_info_print_table_header(2, "study support", "enabled");
    php_info_print_table_end();
}


ZEND_BEGIN_ARG_INFO_EX(arginfo_study_coroutine_create, 0, 0, 1)
                ZEND_ARG_CALLABLE_INFO(0, func, 0)
ZEND_END_ARG_INFO()

PHP_FUNCTION (study_coroutine_create);

ZEND_BEGIN_ARG_INFO_EX(arginfo_study_coroutine_void, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_FUNCTION (study_event_init) {
    int ret;
    ret = st_event_init();
    if (ret < 0) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_FUNCTION (study_event_wait) {
    int ret;
    ret = st_event_wait();
    if (ret < 0) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}


const zend_function_entry study_functions[] = {
        PHP_FE(study_coroutine_create, arginfo_study_coroutine_create)
//      PHP_FALIAS的作用就是用来取别名的，这里换成了我们需要的短名sgo
        PHP_FALIAS(sgo, study_coroutine_create, arginfo_study_coroutine_create)
        PHP_FE(study_timer_test, NULL) // 新增加的一行
        PHP_FE(study_event_init, arginfo_study_coroutine_void)
        PHP_FE(study_event_wait, arginfo_study_coroutine_void)

        PHP_FE_END
};

zend_module_entry study_module_entry = {
        STANDARD_MODULE_HEADER,
        "study",
        study_functions,
        PHP_MINIT(study),
        PHP_MSHUTDOWN(study),
        PHP_RINIT(study),
        PHP_RSHUTDOWN(study),
        PHP_MINFO(study),
        PHP_STUDY_VERSION,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_STUDY
ZEND_GET_MODULE(study)
#endif
