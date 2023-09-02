#!/bin/sh

valgrind -v --num-callers=20 --leak-check=yes --leak-resolution=high --show-reachable=yes $@
