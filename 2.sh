#!/bin/bash
## make clean 只有重新执行了configure才需要执行
make clean && \
make -j 8 && \
make install