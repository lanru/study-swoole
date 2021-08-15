<?php

function deferFunc1()
{
    echo "in defer deferFunc1" . PHP_EOL;
}

function deferFunc2()
{
    echo "in defer deferFunc2" . PHP_EOL;
}

function deferFunc3()
{
    echo "in defer deferFunc3" . PHP_EOL;
}

function task()
{
    echo "task coroutine start" . PHP_EOL;
    SCo::defer('deferFunc1');
    SCo::defer('deferFunc2');
    SCo::defer('deferFunc3');
    echo "task coroutine end" . PHP_EOL;
}

$type = $argv[1];

if ($type==1){
//方式1,类用全名的方式调用 study_coroutine_util_init方法中INIT_NS_CLASS_ENTRY(study_coroutine_ce, "Study", "Coroutine", study_coroutine_util_methods);
echo $type.PHP_EOL;
$cid1 = Study\Coroutine::create('task');
}elseif($type==2){
echo $type.PHP_EOL;
//方式2,类用别名的方式调用,study_coroutine_util_init中zend_register_class_alias("SCo", study_coroutine_ce_ptr); // 新增的代码
$cid1 = SCo::create('task');
}elseif($type==3){
echo $type.PHP_EOL;
//方式3,函数方式, study_functions中PHP_FE(study_coroutine_create, arginfo_study_coroutine_create)
$cid1 = study_coroutine_create('task');
}elseif($type==4){
echo $type.PHP_EOL;
//方式4,study_functions中PHP_FALIAS(sgo, study_coroutine_create, arginfo_study_coroutine_create)
 $cid1 = sgo('task');
}








print_r($argv);
