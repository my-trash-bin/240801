#!/bin/sh

gcc -o run.exe src/main.c src/bmp.c -lEGL -lGLESv2
xvfb-run ./run.exe
