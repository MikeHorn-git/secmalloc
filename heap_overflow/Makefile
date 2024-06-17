CC=g++
CXXFLAGS=-ldl -Wall -Werror -Wextra
PY=python3
PRJ = heap_overflow
CXXPRJ = ${PRJ}.cc
PYPRJ = overflow.py
DIR = ../example

all: build lib python

build:
	${CC} ${CXXFLAGS} ${CXXPRJ} -o ${PRJ}

lib:
	$(MAKE) -C $(DIR) dynamic

python:
	${PY} ${PYPRJ}

clean:
	${RM} ${PRJ} ./*.db 

.PHONY: all build lib python clean
