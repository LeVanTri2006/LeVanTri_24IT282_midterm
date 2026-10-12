CC = cc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L -I./include

OBJDIR = obj
TARGET = myls
OBJS = obj/main.o obj/options.o obj/list.o obj/display.o obj/sort.o obj/util.o

all: ${OBJDIR} ${TARGET}

${OBJDIR}:
	mkdir -p ${OBJDIR}

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} -o ${TARGET} ${OBJS}

obj/main.o: src/main.c ${OBJDIR}
	${CC} ${CFLAGS} -c src/main.c -o obj/main.o

obj/options.o: src/options.c ${OBJDIR}
	${CC} ${CFLAGS} -c src/options.c -o obj/options.o

obj/list.o: src/list.c ${OBJDIR}
	${CC} ${CFLAGS} -c src/list.c -o obj/list.o

obj/display.o: src/display.c ${OBJDIR}
	${CC} ${CFLAGS} -c src/display.c -o obj/display.o

obj/sort.o: src/sort.c ${OBJDIR}
	${CC} ${CFLAGS} -c src/sort.c -o obj/sort.o

obj/util.o: src/util.c ${OBJDIR}
	${CC} ${CFLAGS} -c src/util.c -o obj/util.o

clean:
	rm -rf ${OBJDIR} ${TARGET}

.PHONY: all clean
