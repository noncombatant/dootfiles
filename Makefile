CC = clang
CFLAGS = -Weverything -Werror -std=c2x -O3 \
	-Wno-poison-system-directories \
	-Wno-padded \
	-Wno-declaration-after-statement \
	-Wno-pre-c2x-compat \
	-Wno-reserved-macro-identifier \
	-Wno-unused-macros \
	-Wno-unsafe-buffer-usage \
	-Wno-deprecated-declarations

all: expand walk
	strip expand
	strip walk

walk: walk.c utils.o

expand: expand.c utils.o

clean:
	-rm -rf expand walk *.dSYM *.o
