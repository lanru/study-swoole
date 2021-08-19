<?php

study_event_init();

$chan = new Study\Coroutine\Channel();

Sgo(function () use ($chan)
{
    $ret = $chan->pop();
    var_dump($ret);
});

Sgo(function () use ($chan)
{
    $ret = $chan->push("hello world");
    var_dump($ret);
});

study_event_wait();
