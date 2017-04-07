#!/bin/bash

cd syscalls
make
cd ..
./createfs -i fsdir -o student-distrib/filesys_img
