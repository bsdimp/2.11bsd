#
# Copyright (c) 1988 Regents of the University of California.
# All rights reserved.
#
# Redistribution and use in source and binary forms are permitted
# provided that the above copyright notice and this paragraph are
# duplicated in all such forms and that any documentation, advertising
# materials, and other materials related to such redistribution and
# use acknowledge that the software was developed by the University
# of California, Berkeley.  The name of the University may not be
# used to endorse or promote products derived from this software
# without specific prior written permission.  THIS SOFTWARE IS PROVIDED
# ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
# WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND
# FITNESS FOR A PARTICULAR PURPOSE.
#
# @(#)Makefile	5.2.2 (2.11BSD) 1996/12/1
#

SEPFLAG= -i
CFLAGS=	-O -DSECSPERDAY=86400 -DDAYSPERNYEAR=365 ${SEPFLAG}
SRCS=	finger.c lprint.c net.c sprint.c util.c
OBJS=	finger.o lprint.o net.o sprint.o util.o
MAN=	finger.0

all: finger ${MAN}

finger: ${OBJS}
	${CC} -o $@ ${CFLAGS} ${OBJS}

clean:
	rm -f ${OBJS} core finger ${MAN}

cleandir: clean
	rm -f ${MAN} tags .depend

depend: ${SRCS}
	mkdep ${CFLAGS} ${SRCS}

# install: ${MAN}
install: finger ${MAN}
	install -s -o bin -g bin -m 755 finger ${DESTDIR}/usr/ucb
	install -c -o bin -g bin -m 444 finger.0 ${DESTDIR}/usr/man/cat1

${MAN}: finger.1
	/usr/man/manroff finger.1 > ${MAN}

lint: ${SRCS}
	lint ${CFLAGS} ${SRCS}

tags: ${SRCS}
	ctags ${SRCS}
