CC = gcc
CFLAGS = -I./include -Wall -Wextra -Werror -D_GNU_SOURCE
PICFLAGS = -fPIC
PRJ = my_secmalloc
OBJS = src/${PRJ}.o
SLIB = lib/lib${PRJ}.a
LIB = lib/lib${PRJ}.so
TEST = test/test

all: dynamic static

${LIB}: ${OBJS} | lib
	$(CC) ${CFLAGS} ${PICFLAGS} -shared -o $@ $^

${SLIB}: ${OBJS} | lib
	ar rcs $@ $^

dynamic: ${LIB}

static: ${SLIB}

lib:
	mkdir -p lib

clean:
	${RM} src/.*.swp src/*~ src/*.o test/*.o lib/* log.txt

distclean: clean
	${RM} ${SLIB} ${LIB} ${TEST}

build_test: CFLAGS += -DTEST
build_test: ${OBJS} test/test.o | lib
	$(CC) -o test/test $^ -lcriterion -Llib

test: build_test
	LD_LIBRARY_PATH=./lib ./test/test || true

.PHONY: all clean build_test dynamic test static distclean

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(PICFLAGS) -c $< -o $@

test/test.o: test/test.c
	$(CC) $(CFLAGS) -c $< -o $@
