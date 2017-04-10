#!/bin/bash

cd ../syscalls
make
cd ..
cd fish
make
cd ..
cp fish/fish fsdir
./createfs -i fsdir -o student-distrib/filesys_img
cd student-distrib
make clean
make
