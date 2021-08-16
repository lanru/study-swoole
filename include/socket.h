//
// Created by wlh on 2021/8/16.
//

#ifndef PHP_7_4_15_SOCKET_H
#define PHP_7_4_15_SOCKET_H

#include "study.h"
enum stSocket_type
{
    ST_SOCK_TCP          =  1,
    ST_SOCK_UDP          =  2,
};
int stSocket_create(int type);

int stSocket_bind(int sock, int type, char *host, int port);

int stSocket_accept(int sock);

#endif //PHP_7_4_15_SOCKET_H
