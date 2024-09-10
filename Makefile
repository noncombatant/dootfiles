CC = clang
CFLAGS = -Weverything -Werror -std=c2x -O3 \
	-Wno-poison-system-directories \
	-Wno-padded \
	-Wno-declaration-after-statement \
	-Wno-pre-c2x-compat \
	-Wno-reserved-macro-identifier \
	-Wno-unused-macros \
	-Wno-unsafe-buffer-usage

all: walk
	strip walk

walk:

clean:
	-rm -f walk
