#!/bin/sh
g++ -o main main.cpp -I. -I/usr/include/lua5.2 -llua5.2 -lsfml-system -lsfml-graphics -lsfml-window
