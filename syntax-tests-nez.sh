cmake --build /home/siy/CLionProjects/CafeBabe/cmake-build-debug --target all -- -j 19
find ./testInputs/ -type f | sort | xargs -iarg nez parse -g CafeBabe.nez arg