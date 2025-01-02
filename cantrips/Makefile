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
	-Wno-unknown-warning-option \
	-Wno-pre-c11-compat \
	-Wno-pre-c2x-compat \
	-Wno-gnu-statement-expression-from-macro-expansion

ifdef RELEASE
	CFLAGS += -O3 -flto=thin
else
	CFLAGS += -O0 -g -DTEST -fsanitize=address -fsanitize=undefined -fsanitize-trap=all
endif

TARGETS = cli_test color expand fold list pathname shuffle walk
.PHONY: all clean strip

all: $(TARGETS)
strip:
	strip $(TARGETS)
color: color.c utils.o
list: list.c utils.o
expand: expand.c utils.o
fold: fold.c utils.o
pathname: pathname.c utils.o
shuffle: shuffle.c utils.o
walk: walk.c utils.o
cli_test: cli_test.c cli.o utils.o
clean:
	-rm -rf $(TARGETS) *.dSYM *.o
