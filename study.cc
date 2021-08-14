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

const zend_function_entry study_functions[] = {
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
