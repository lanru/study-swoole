<?php

Sgo(function ()
{
    $serv = new Study\Coroutine\Server("127.0.0.1", 8080);
    $connfd = $serv->accept();
    var_dump($connfd);
});
Sco::scheduler();