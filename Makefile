C=gcc
CFLAGS=-Wall -Wpedantic -Wextra -O3

TARGET=battle
all: ${TARGET}
.PHONY: all, clean

${TARGET}: src/main.c src/battle.c 
		${C} ${CFLAGS} -o $@ $^ -Iinclude

clean:
		rm -f ${TARGET}
