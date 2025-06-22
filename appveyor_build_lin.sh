#!/bin/bash

set -e
set -x

echo CI_DEBUG_CONFIGURATION: $CONFIGURATION

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=$CONFIGURATION -DCMAKE_VERBOSE_MAKEFILE=ON ..
cmake --build .
ctest
