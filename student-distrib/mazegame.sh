#!/bin/bash

cd ../mazegame
make
cd ..
cp mazegame/mazegame fsdir
./createfs -i fsdir -o student-distrib/filesys_img
cd student-distrib
make clean
make
