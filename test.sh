#!/bin/bash
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --show-leak-kinds=all --track-origins=yes --log-file=valgrind-out.txt ./server 8080
