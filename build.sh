#!/bin/bash
tcc -Wall -Werror -Wextra *.c -o server -lsqlite3
