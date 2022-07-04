# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -std=gnu11 -pedantic -Wall \
-Wconversion -Wimplicit-function-declaration \
-Wno-missing-braces -Wextra -Wno-missing-field-initializers -Wformat=2 \
-Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith \
-Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline \
-Wundef -Wnested-externs -Wcast-qual -Wshadow -Wunreachable-code \
-Wfloat-equal -Wstrict-aliasing=2 -Wredundant-decls \
-Wold-style-definition -Wno-unused-result -Wno-unused-parameter\
-ggdb3 \
-O3 \
-DTEST_LIBRARY \
-fno-omit-frame-pointer -fno-common -fstrict-aliasing -fshort-enums


LIBRARY_FN = lib_cvorg_protocol.so
SOURCE_FILES = ../src/cvorg_protocol.c
HEADER_FILES = ../include/cvorg_protocol.h
INCLUDE_DIRS = ../include

UNAME := $(shell uname -s)

ifneq (,$(findstring MINGW,$(UNAME)))
	UNAME := MinGW
	LDFLAGS = -lm
else
	LDFLAGS = -lm -lc
endif

# the build target executable:

all: library

library:

ifeq ($(UNAME),Darwin)
	$(CC) -I$(INCLUDE_DIRS) -arch x86_64 -Wc11-extensions -Wshorten-64-to-32 $(CFLAGS) -fPIC -shared $(SOURCE_FILES) -Wl,-install_name,$(LIBRARY_FN) -o $(LIBRARY_FN) $(LDFLAGS)
endif

ifeq ($(UNAME),Linux)
	$(CC) -I$(INCLUDE_DIRS) $(CFLAGS) -fPIC -shared $(SOURCE_FILES) -o $(LIBRARY_FN) $(LDFLAGS)
endif

ifeq ($(UNAME),MinGW)
	$(CC) -I$(INCLUDE_DIRS) $(CFLAGS) -fPIC -shared $(SOURCE_FILES) -o $(LIBRARY_FN) $(LDFLAGS)
endif

#	$(CC) -I$(INCLUDE_DIRS) $(CFLAGS) $(HEADER_FILES) -E | grep -v '^\#' > lib_cvorg_protocol.i.h

.PHONY: clean
clean:
	$(RM) $(TARGET)
