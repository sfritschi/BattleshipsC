C=gcc
CFLAGS=-Wall -Wpedantic -Wextra -O3

TARGET=battle
all: ${TARGET}
.PHONY: all, clean

${TARGET}: main.c battle.c battle.h
		${C} ${CFLAGS} -o $@ $^

clean:
		rm -f ${TARGET}
