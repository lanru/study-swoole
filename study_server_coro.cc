//
// Created by wlh on 2021/8/16.
//
#include "study_server_coro.h"

using study::coroutine::Socket;
/**
 * Define zend class entry
 */
zend_class_entry study_coroutine_server_coro_ce;
zend_class_entry *study_coroutine_server_coro_ce_ptr;

ZEND_BEGIN_ARG_INFO_EX(arginfo_study_coroutine_server_coro_construct,
                       0, 0, 2)
                ZEND_ARG_INFO(0, host)
                ZEND_ARG_INFO(0, port)

ZEND_END_ARG_INFO()

PHP_METHOD (study_coroutine_server_coro, __construct) {
    zval *zhost;
    zend_long zport;
    zval zsock;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_ZVAL(zhost)
            Z_PARAM_LONG(zport)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    Socket *sock = new Socket(AF_INET, SOCK_STREAM, 0);

    sock->bind(ST_SOCK_TCP, Z_STRVAL_P(zhost), zport);
    sock->listen();

    ZVAL_PTR(&zsock, sock);

    zend_update_property(study_coroutine_server_coro_ce_ptr, getThis(), ZEND_STRL("zsock"), &zsock);
    zend_update_property_string(study_coroutine_server_coro_ce_ptr, getThis(), ZEND_STRL("host"), Z_STRVAL_P(zhost));
    zend_update_property_long(study_coroutine_server_coro_ce_ptr, getThis(), ZEND_STRL("port"), zport);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_study_coroutine_void, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_METHOD (study_coroutine_server_coro, accept) {
    zval *zsock;
    Socket *sock; // 新增的一行
    int connfd;

    zsock = st_zend_read_property(study_coroutine_server_coro_ce_ptr, getThis(), ZEND_STRL("zsock"), 0); // 修改的一行
    sock = (Socket *) Z_PTR_P(zsock); // 修改的一行
    connfd = sock->accept(); // 修改的一行
    RETURN_LONG(connfd);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_study_coroutine_server_coro_recv, 0, 0, 2)
                ZEND_ARG_INFO(0, fd)
                ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

PHP_METHOD (study_coroutine_server_coro, recv) {
    int ret;
    zend_long fd;
    zend_long length = 65536;

    ZEND_PARSE_PARAMETERS_START(1, 2)
            Z_PARAM_LONG(fd)
            Z_PARAM_OPTIONAL
            Z_PARAM_LONG(length)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    zend_string *buf = zend_string_alloc(length, 0);

    ret = stSocket_recv(fd, ZSTR_VAL(buf), length, 0);
    if (ret == 0) {
        zend_update_property_long(study_coroutine_server_coro_ce_ptr, getThis(), ZEND_STRL("errCode"),
                                  ST_ERROR_SESSION_CLOSED_BY_CLIENT);
        zend_update_property_string(study_coroutine_server_coro_ce_ptr, getThis(), ZEND_STRL("errMsg"),
                                    st_strerror(ST_ERROR_SESSION_CLOSED_BY_CLIENT));
        RETURN_FALSE;
    }
    if (ret < 0) {
        php_error_docref(NULL, E_WARNING, "recv error");
        RETURN_FALSE;
    }
    ZSTR_VAL(buf)[ret] = '\0';
    RETURN_STR(buf);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_study_coroutine_server_coro_send, 0, 0, 2)
                ZEND_ARG_INFO(0, fd)
                ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

PHP_METHOD (study_coroutine_server_coro, send) {
    ssize_t retval;
    zend_long fd;
    char *data;
    size_t length;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_LONG(fd)
            Z_PARAM_STRING(data, length)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    retval = stSocket_send(fd, data, length, 0);
    if (retval < 0) {
        php_error_docref(NULL, E_WARNING, "send error");
        RETURN_FALSE;
    }
    RETURN_LONG(retval);
}


static const zend_function_entry study_coroutine_server_coro_methods[] =
        {
                PHP_ME(study_coroutine_server_coro, __construct, arginfo_study_coroutine_server_coro_construct,
                       ZEND_ACC_PUBLIC |
                       ZEND_ACC_CTOR) // ZEND_ACC_CTOR is used to declare that this method is a constructor of this class.
                PHP_ME(study_coroutine_server_coro, accept, arginfo_study_coroutine_void, ZEND_ACC_PUBLIC)
                PHP_ME(study_coroutine_server_coro, recv, arginfo_study_coroutine_server_coro_recv, ZEND_ACC_PUBLIC)
                PHP_ME(study_coroutine_server_coro, send, arginfo_study_coroutine_server_coro_send, ZEND_ACC_PUBLIC)
                PHP_FE_END
        };


void study_coroutine_server_coro_init() {
    INIT_NS_CLASS_ENTRY(study_coroutine_server_coro_ce, "Study", "Coroutine\\Server",
                        study_coroutine_server_coro_methods);
    study_coroutine_server_coro_ce_ptr = zend_register_internal_class(&study_coroutine_server_coro_ce
                                                                      TSRMLS_CC); // Registered in the Zend Engine

    zval *zsock = (zval *) malloc(sizeof(zval));
    zend_declare_property(study_coroutine_server_coro_ce_ptr, ZEND_STRL("zsock"), zsock, ZEND_ACC_PUBLIC);
    zend_declare_property_string(study_coroutine_server_coro_ce_ptr, ZEND_STRL("host"), "", ZEND_ACC_PUBLIC);
    zend_declare_property_long(study_coroutine_server_coro_ce_ptr, ZEND_STRL("port"), -1, ZEND_ACC_PUBLIC);
    // 省略了其他的代码
    zend_declare_property_long(study_coroutine_server_coro_ce_ptr, ZEND_STRL("errCode"), 0, ZEND_ACC_PUBLIC);
    zend_declare_property_string(study_coroutine_server_coro_ce_ptr, ZEND_STRL("errMsg"), "", ZEND_ACC_PUBLIC);
}

