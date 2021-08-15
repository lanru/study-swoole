PHP_ARG_ENABLE(study, whether to enable study support,
Make sure that the comment is aligned:
[  --enable-study           Enable study support])

# AC_CANONICAL_HOST

if test "$PHP_STUDY" != "no"; then
########新增加的2行 用来添加链接库，并且指明我们的动态链接库的路径，增加了config.m4文件的这两行之后，会在./configure命令生成的Makefile里面得到体现
    PHP_ADD_LIBRARY_WITH_PATH(uv, /usr/local/lib/, STUDY_SHARED_LIBADD)
    PHP_SUBST(STUDY_SHARED_LIBADD)
########新增加的2行

    PHP_ADD_LIBRARY(pthread)
    STUDY_ASM_DIR="thirdparty/boost/asm/"
    CFLAGS="-Wall -pthread $CFLAGS"

    AS_CASE([$host_os],
      [linux*], [SW_OS="LINUX"],
      [darwin*], [SW_OS="MAC"],
      []
    )
        SW_ASM_DIR="thirdparty/boost/asm/"
        SW_USE_ASM_CONTEXT="yes"

  AS_CASE([$host_cpu],
     [x86_64*], [SW_CPU="x86_64"],
     [amd64*], [SW_CPU="x86_64"],
     [x86*], [SW_CPU="x86"],
     [i?86*], [SW_CPU="x86"],
     [arm*], [SW_CPU="arm"],
     [aarch64*], [SW_CPU="arm64"],
     [arm64*], [SW_CPU="arm64"],
     [mips*], [SW_CPU="mips32"],
     [
       SW_USE_ASM_CONTEXT="no"
     ]
   )

    if test "$SW_OS" = "MAC"; then
        SW_CONTEXT_ASM_FILE="combined_sysv_macho_gas.S"
    elif test "$SW_CPU" = "x86_64"; then
        if test "$SW_OS" = "LINUX" || test "$SW_OS" = "BSD"; then
            SW_CONTEXT_ASM_FILE="x86_64_sysv_elf_gas.S"
        else
            SW_USE_ASM_CONTEXT="no"
        fi
    elif test "$SW_CPU" = "x86"; then
        if test "$SW_OS" = "LINUX" || test "$SW_OS" = "BSD"; then
            SW_CONTEXT_ASM_FILE="i386_sysv_elf_gas.S"
        else
            SW_USE_ASM_CONTEXT="no"
        fi
    elif test "$SW_CPU" = "arm"; then
        if test "$SW_OS" = "LINUX" || test "$SW_OS" = "BSD"; then
            SW_CONTEXT_ASM_FILE="arm_aapcs_elf_gas.S"
        else
            SW_USE_ASM_CONTEXT="no"
        fi
    elif test "$SW_CPU" = "arm64"; then
        if test "$SW_OS" = "LINUX" || test "$SW_OS" = "BSD"; then
            SW_CONTEXT_ASM_FILE="arm64_aapcs_elf_gas.S"
        else
            SW_USE_ASM_CONTEXT="no"
        fi
     elif test "$SW_CPU" = "ppc32"; then
        if test "$SW_OS" = "LINUX"; then
            SW_CONTEXT_ASM_FILE="ppc32_sysv_elf_gas.S"
        else
            SW_USE_ASM_CONTEXT="no"
        fi
    elif test "$SW_CPU" = "ppc64"; then
        if test "$SW_OS" = "LINUX" || test "$SW_OS" = "BSD"; then
            SW_CONTEXT_ASM_FILE="ppc64_sysv_elf_gas.S"
        else
            SW_USE_ASM_CONTEXT="no"
        fi
    elif test "$SW_CPU" = "mips32"; then
        if test "$SW_OS" = "LINUX"; then
           SW_CONTEXT_ASM_FILE="mips32_o32_elf_gas.S"
        else
            SW_USE_ASM_CONTEXT="no"
        fi
    else
        SW_USE_ASM_CONTEXT="no"
    fi

    CFLAGS="-Wall -pthread $CFLAGS"
    LDFLAGS="$LDFLAGS -lpthread"

    if test "$SW_OS" = "MAC"; then
        AC_CHECK_LIB(c, clock_gettime, AC_DEFINE(HAVE_CLOCK_GETTIME, 1, [have clock_gettime]))
    else
        AC_CHECK_LIB(rt, clock_gettime, AC_DEFINE(HAVE_CLOCK_GETTIME, 1, [have clock_gettime]))
        PHP_ADD_LIBRARY(rt, 1, SWOOLE_SHARED_LIBADD)
    fi
    if test "$SW_OS" = "LINUX"; then
        LDFLAGS="$LDFLAGS -z now"
    fi

    AC_MSG_CHECKING([if compiling with clang])
    AC_COMPILE_IFELSE([
        AC_LANG_PROGRAM([], [[
            #ifndef __clang__
                not clang
            #endif
        ]])],
        [CLANG=yes], [CLANG=no]
    )
    AC_MSG_RESULT([$CLANG])

    if test "$CLANG" = "yes"; then
        CFLAGS="$CFLAGS -std=gnu89"
    fi

 dnl 这段是把我们需要编译的所有文件已字符串的方式存入到变量study_source_file里面
    study_source_file="\
        study.cc \
        study_coroutine.cc \
        study_coroutine_util.cc \
        src/coroutine/coroutine.cc \
        src/coroutine/context.cc \
        ${STUDY_ASM_DIR}make_${SW_CONTEXT_ASM_FILE} \
        ${STUDY_ASM_DIR}jump_${SW_CONTEXT_ASM_FILE} \
        study_server_coro.cc \
        src/socket.cc
    "
     AC_DEFINE(SW_USE_ASM_CONTEXT, 1, [use boost asm context])
 dnl 这段是声明这个扩展的名称、需要的源文件名、此扩展的编译形式。其中$ext_shared代表此扩展是动态库，使用cxx的原因是，我们的这个扩展使用C++来编写
    PHP_NEW_EXTENSION(study, $study_source_file, $ext_shared, ,, cxx)
 dnl 这段是用来添加额外的包含头文件的目录。
    PHP_ADD_INCLUDE([$ext_srcdir])
    PHP_ADD_INCLUDE([$ext_srcdir/include])
 dnl 这段是把我们的study扩展目录里面的*.h、config.h、include/*.h、thirdparty/*.h复制到php-config --include-dir，我的电脑上是/Users/wlh/Documents/study/clion/php-7.4.15/work/include/php这个目录
    PHP_INSTALL_HEADERS([ext/study], [*.h config.h include/*.h thirdparty/*.h])
 dnl 因为，我们使用了C++，所以我们需要指明一下。（没有这句会编译出错）
    PHP_REQUIRE_CXX()
 dnl 这段是指定编译C++时候，用到的编译选项
    CXXFLAGS="$CXXFLAGS -Wall -Wno-unused-function -Wno-deprecated -Wno-deprecated-declarations"
    CXXFLAGS="$CXXFLAGS -std=c++11"
 dnl 这段是指定这个扩展需要被编译到的文件的目录。因为我们需要编译boost提供的代码，所以需要进行指定
    PHP_ADD_BUILD_DIR($ext_builddir/thirdparty/boost)
    PHP_ADD_BUILD_DIR($ext_builddir/thirdparty/boost/asm)
fi
