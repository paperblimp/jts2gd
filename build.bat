mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G "NMake Makefiles"
nmake
move jts2gd.exe ..
cd ..