#include "php_study.h"

PHP_MINIT_FUNCTION(study)
{
    // 协程类注册
    study_coroutine_util_init();
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(study)
{
    return SUCCESS;
}

PHP_RINIT_FUNCTION(study)
{
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(study)
{
    return SUCCESS;
}

PHP_MINFO_FUNCTION(study)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "study support", "enabled");
    php_info_print_table_end();
}


ZEND_BEGIN_ARG_INFO_EX(arginfo_study_coroutine_create, 0, 0, 1)
                ZEND_ARG_CALLABLE_INFO(0, func, 0)
ZEND_END_ARG_INFO()

PHP_FUNCTION(study_coroutine_create);


const zend_function_entry study_functions[] = {
        PHP_FE(study_coroutine_create, arginfo_study_coroutine_create)
//      PHP_FALIAS的作用就是用来取别名的，这里换成了我们需要的短名sgo
        PHP_FALIAS(sgo, study_coroutine_create, arginfo_study_coroutine_create)
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
