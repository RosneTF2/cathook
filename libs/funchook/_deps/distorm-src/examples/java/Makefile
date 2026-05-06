# /^-----^\   data: 2026-04-30
# V  o o  V  plik: libs/funchook/_deps/distorm-src/examples/java/Makefile
#  |  Y  |   autor: pupnoodle
#   \ Q /
#   / - \
#   |    \
#   |     \     )
#   || (___\====

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)

JAVA_HOME=$(shell /usr/libexec/java_home)

all: libjdistorm.dylib
libjdistorm.dylib: jdistorm.c jdistorm.h
	gcc -dynamiclib -o libjdistorm.dylib jdistorm.c -I ${JAVA_HOME}/include/ -I ${JAVA_HOME}/include/darwin/ -ldistorm3

endif

ifeq ($(UNAME_S),Linux)

all: libjdistorm.so
jdistorm.o: jdistorm.c jdistorm.h
        gcc -c jdistorm.c -fPIC -I ${JAVA_HOME}/include -I ${JAVA_HOME}/include/linux

libjdistorm.so: jdistorm.o
        gcc -shared -o libjdistorm.so -L${JAVA_HOME}/jre/lib -ldistorm3 jdistorm.o

endif

