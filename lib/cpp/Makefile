#	Makefile	1.4	(2.11BSD) 2001/11/19
#
CFLAGS=-O -Dpdp11=1 -DFLEXNAMES -DBSD2_11 -UBSD2_10
SEPFLAG=-i

cpp :	cpp.o cpy.o
	$(CC) $(CFLAGS) $(SEPFLAG) -o cpp cpp.o cpy.o

cpy.c : cpy.y
	yacc cpy.y
	mv y.tab.c cpy.c

cpy.o :	cpy.c yylex.c

clean :
	rm -f *.o cpy.c cpp

install : cpp
	install -s cpp $(DESTDIR)/lib
