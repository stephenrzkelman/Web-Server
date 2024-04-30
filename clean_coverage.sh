#!/bin/bash

rm -rf build_coverage;
mkdir build_coverage && 
cd build_coverage &&
cmake -DCMAKE_BUILD_TYPE=coverage .. &&
make coverage;
cd ..;
