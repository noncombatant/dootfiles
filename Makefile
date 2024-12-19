CC = clang
CFLAGS = -Weverything -Werror -std=c2x \
	-Wno-poison-system-directories \
	-Wno-padded \
	-Wno-declaration-after-statement \
	-Wno-pre-c2x-compat \
	-Wno-reserved-macro-identifier \
	-Wno-unused-macros \
	-Wno-unsafe-buffer-usage \
	-Wno-deprecated-declarations \
	-Wno-pre-c11-compat

ifdef RELEASE
	CFLAGS += -O3 -flto=thin
else
	CFLAGS += -O0 -g -fsanitize=address -fsanitize=undefined -fsanitize-trap=all
endif

all: color expand walk
	strip color
	strip expand
	strip walk

walk: walk.c utils.o

expand: expand.c utils.o

color: color.c utils.o

clean:
	-rm -rf color expand walk *.dSYM *.o
