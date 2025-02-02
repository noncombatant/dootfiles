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
	-Wno-gnu-statement-expression-from-macro-expansion \
	-Wno-switch-default

ifdef RELEASE
	CFLAGS += -O3 -flto=thin
else
	CFLAGS += -O0 -g -DTEST -fsanitize=address -fsanitize=undefined -fsanitize-trap=all
endif

TARGETS = cli_test clocks color expand fold list pathname shuffle walk
.PHONY: all clean strip

all: $(TARGETS)
strip: $(TARGETS)
	strip $(TARGETS)
clocks: clocks.c utils.o
color: color.c utils.o
list: list.c cli.o utils.o
expand: expand.c cli.o utils.o
fold: fold.c cli.o utils.o
pathname: pathname.c cli.o utils.o
shuffle: shuffle.c cli.o utils.o
walk: walk.c utils.o
cli_test: cli_test.c cli.o utils.o
clean:
	-rm -rf $(TARGETS) *.dSYM *.o
