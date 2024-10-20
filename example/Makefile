CC = gcc
CFLAGS = -I./include -Wall -Wextra -Werror -D_GNU_SOURCE
PICFLAGS = -fPIC
PRJ = my_secmalloc
OBJS = src/${PRJ}.o
SLIB = lib/lib${PRJ}.a
LIB = lib/lib${PRJ}.so
TEST = test/test

all: dynamic static

help:
	@echo "Usage: make <target>"
	@echo "Targets:"
	@echo "  all           Build both dynamic and static libraries"
	@echo "  dynamic       Build the dynamic library (.so)"
	@echo "  static        Build the static library (.a)"
	@echo "  test          Build and run unit tests"
	@echo "  clean         Remove object files and temporary files"
	@echo "  distclean     Clean up all built files and libraries"
	@echo "  build_test    Build the test executable"
	@echo "  help          Display this help message"

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

.PHONY: all help clean build_test dynamic test static distclean

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(PICFLAGS) -c $< -o $@

test/test.o: test/test.c
	$(CC) $(CFLAGS) -c $< -o $@
