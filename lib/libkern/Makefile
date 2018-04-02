#
#	@(#)Makefile	1.3 (2.11BSD) 2000/11/13
#
# This is the Makefile for 'libkern.a'.  These are the specially compiled
# versions of libc.a routines which the kernel and network use.
#
# There are no source files in this directory.  Links are made to the files.
#
# libkern.a needs to be installed only once (usually when the 
# C library is installed) rather than compiling the sources each time 
# a kernel is compiled..

DEFS=	-DKERNEL
CFLAGS=	 -O ${DEFS} -I../libc/pdp/gen
CPP=	/lib/cpp -P ${DEFS} -I../libc/pdp/gen
AS=	/bin/as -u
PREFIX=	/usr/src/lib/libc/pdp

SRCS=	bcmp.s bcopy.s bzero.s ffs.s htonl.s htons.s insque.s \
	ldiv.s lmul.s lrem.s remque.s strlen.s udiv.s uldiv.s ulsh.s ulrem.s \
	almul.s strcpy.s strncpy.s strcat.s index.s strncmp.s strcmp.s \
	atoi.c atol.c

OBJS=	bcmp.o bcopy.o bzero.o ffs.o htonl.o htons.o insque.o \
	ldiv.o lmul.o lrem.o remque.o strlen.o udiv.o uldiv.o ulsh.o ulrem.o \
	almul.o strcpy.o strncpy.o strcat.o index.o strncmp.o strcmp.o \
	atoi.o atol.o

.c.o:
	${CC} ${CFLAGS} -c $*.c
	ld -X -r $*.o
	mv a.out $*.o

.s.o:
	${CPP} $*.s | ${AS} -o $*.o
	ld -X -r $*.o
	mv a.out $*.o

all: FRC libkern.a

libkern.a: ${OBJS}
	rm -f libkern.a
	ar rv libkern.a ${OBJS}

clean:
	rm -f ${OBJS} ${SRCS} libkern.a

depend: ${SRCS}
	mkdep ${CFLAGS} ${SRCS}

install: FRC libkern.a
	install -o root -g bin -m 644 libkern.a ${DESTDIR}/usr/lib/libkern.a
	ranlib ${DESTDIR}/usr/lib/libkern.a

FRC:
	rm -f ${SRCS}
	ln ${PREFIX}/gen/bcmp.s bcmp.s 
	ln ${PREFIX}/gen/bcopy.s bcopy.s
	ln ${PREFIX}/gen/bzero.s bzero.s
	ln ${PREFIX}/gen/ffs.s ffs.s
	ln ${PREFIX}/gen/insque.s insque.s
	ln ${PREFIX}/gen/remque.s remque.s
	ln ${PREFIX}/gen/strlen.s strlen.s
	ln ${PREFIX}/crt/ldiv.s ldiv.s
	ln ${PREFIX}/crt/lmul.s lmul.s
	ln ${PREFIX}/crt/almul.s almul.s
	ln ${PREFIX}/crt/lrem.s lrem.s
	ln ${PREFIX}/crt/udiv.s udiv.s
	ln ${PREFIX}/crt/uldiv.s uldiv.s
	ln ${PREFIX}/crt/ulrem.s ulrem.s
	ln ${PREFIX}/crt/ulsh.s ulsh.s
	ln ${PREFIX}/net/htonl.s htonl.s
	ln ${PREFIX}/net/htons.s htons.s
	ln ${PREFIX}/gen/strcpy.s strcpy.s
	ln ${PREFIX}/gen/strncpy.s strncpy.s
	ln ${PREFIX}/gen/strcat.s strcat.s
	ln ${PREFIX}/gen/index.s index.s
	ln ${PREFIX}/gen/strncmp.s strncmp.s
	ln ${PREFIX}/gen/strcmp.s strcmp.s
	ln ${PREFIX}/../gen/atoi.c atoi.c
	ln ${PREFIX}/../gen/atol.c atol.c

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.
