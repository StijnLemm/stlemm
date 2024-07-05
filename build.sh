#! /bin/zsh

echo '[INFO] Starting build:'
mkdir -p ./build/ > /dev/null
if [ $# = 1 ]; then
    if [ $1 = '32' ]; then
	echo  '[INFO] Building 32-bit!'
	time clang++ -Wno-deprecated-declarations -std=c++17 main.cpp -m32 -o ./build/program_32bit && ./build/program_32bit
	exit
    fi
fi

echo '[INFO] Building 64-bit!'
time clang++ -Wno-deprecated-declarations -std=c++17 main.cpp -o ./build/program && ./build/program

echo '[INFO] Done'

