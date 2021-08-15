<?php

function deferFunc()
{
    echo "in defer method" . PHP_EOL;
}

function task()
{
    echo "task coroutine start" . PHP_EOL;
    Study\Coroutine::defer('deferFunc');
    echo "task coroutine end" . PHP_EOL;
}

$cid1 = Study\Coroutine::create('task');
