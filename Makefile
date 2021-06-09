C=gcc
CFLAGS=-Wall -Wpedantic -Wextra -O3

TARGET=battle
SOURCE=src
HEADER=include

all: ${TARGET}
.PHONY: all, clean

${TARGET}: ${SOURCE}/*.c 
		${C} ${CFLAGS} -o $@ $^ -I${HEADER}

clean:
		rm -f ${TARGET}
