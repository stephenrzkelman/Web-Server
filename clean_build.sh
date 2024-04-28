#!/bin/bash

rm -rf build;
mkdir build && 
cd build && 
cmake .. && 
make && 
make test;
cd ..;

rm -rf build_coverage;
mkdir build_coverage && 
cd build_coverage &&
cmake -DCMAKE_BUILD_TYPE=coverage .. &&
make coverage;
cd ..;
