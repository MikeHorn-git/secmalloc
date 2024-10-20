# Description

School Project

# Makefile

```Makefile
Usage: make <target>
Targets:
  all           Build both dynamic and static libraries
  dynamic       Build the dynamic library (.so)
  static        Build the static library (.a)
  test          Build and run unit tests
  clean         Remove object files and temporary files
  distclean     Clean up all built files and libraries
  build_test    Build the test executable
  help          Display this help message
```

# Usage

```bash
LD_PRELOAD=./libmy_secmalloc.so ls
```

# Authors

* [Itayon](https://github.com/Itayon)
* [MikeHorn-git](https://github.com/MikeHorn-git)
