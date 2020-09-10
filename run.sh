#!/bin/bash

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=./build/Debug/output/valgrind.log ./build/Debug/output/unittest-common
