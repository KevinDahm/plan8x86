#!/bin/bash

(echo 'target remote 10.0.2.2:1234'; cat) | gdb bootimg
