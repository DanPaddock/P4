# Makefile for UD CISC user-level thread library

CC = gcc
CFLAGS = -g

LIBOBJS = t_lib.o 

TSTOBJS = test01x.o test01.o philosopher.o Shone-Hoffman.o T3.o T5.o T6.o T8.o T9.o T11.o

# specify the executable 

EXECS = test01x test01 T3 philosopher Shone-Hoffman T5 T6 T8 T9 T11

# specify the source files

LIBSRCS = t_lib.c

TSTSRCS = test01x.c test01.c philosopher.c Shone-Hoffman.c T3.c T5.c T6.c T8.c T9.c T11.c

# ar creates the static thread library

t_lib.a: ${LIBOBJS} Makefile
	ar rcs t_lib.a ${LIBOBJS}

# here, we specify how each file should be compiled, what
# files they depend on, etc.

t_lib.o: t_lib.c t_lib.h Makefile
	${CC} ${CFLAGS} -c t_lib.c

test01.o: test01.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test01.c
	
test01: test01.o t_lib.a Makefile
	${CC} ${CFLAGS} test01.o t_lib.a -o test01

test01x.o: test01x.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test01x.c

test01x: test01x.o t_lib.a Makefile
	${CC} ${CFLAGS} test01x.o t_lib.a -o test01x

philosopher.o: philosopher.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c philosopher.c

philosopher: philosopher.o t_lib.a Makefile
	${CC} ${CFLAGS} philosopher.o t_lib.a -o philosopher

Shone-Hoffman.o: Shone-Hoffman ud_thread.h Makefile
	${CC} ${CFLAGS} -c Shone-Hoffman.c

Shone-Hoffman: Shone-Hoffman.o t_lib.a Makefile
	${CC} ${CFLAGS} Shone-Hoffman.o t_lib.a -o Shone-Hoffman

T3.o: T3.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c T3.c

T3: T3.o t_lib.a Makefile
	${CC} ${CFLAGS} T3.o t_lib.a -o T3

T5.o: T5.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c T5.c

T5: T5.o t_lib.a Makefile
	${CC} ${CFLAGS} T5.o t_lib.a -o T5

T6.o: T6.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c T6.c

T6: T6.o t_lib.a Makefile
	${CC} ${CFLAGS} T6.o t_lib.a -o T6

T8.o: T8.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c T8.c

T8: T8.o t_lib.a Makefile
	${CC} ${CFLAGS} T8.o t_lib.a -o T8

T9.o: T9.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c T9.c

T9: T9.o t_lib.a Makefile
	${CC} ${CFLAGS} T9.o t_lib.a -o T9

T11.o: T11.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c T11.c

T11: T11.o t_lib.a Makefile
	${CC} ${CFLAGS} T11.o t_lib.a -o T11

clean:
	rm -f t_lib.a ${EXECS} ${LIBOBJS} ${TSTOBJS} 
