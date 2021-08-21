SRC = main.c parser.c
OBJS = ${SRC:.c=.o}
CFLAGS = -g -Wall -Werror -I .

wish: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^

%.o: %.c macros.h
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm -f ${OUT} *.o vgcore.* wish
