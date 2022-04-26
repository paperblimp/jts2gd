#!/bin/bash

if !([ -e "build" ] && [ -d "build" ])
then
    mkdir build;
fi

cd build;
cmake .. -DCMAKE_BUILD_TYPE=Release;
make;
cp jts2gd ..;
cd ..;
