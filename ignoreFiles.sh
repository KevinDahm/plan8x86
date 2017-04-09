#!/bin/bash

cd syscalls/
git update-index --assume-unchanged $(git ls-files | tr '\n' ' ')
cd ../fsdir/
git update-index --assume-unchanged $(git ls-files | tr '\n' ' ')
cd ../student-distrib/
git update-index --assume-unchanged bootimg filesys_img mp3.img
