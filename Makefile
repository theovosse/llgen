CC=gcc -Wall -pedantic
CFLAGS=-g
LEX=/usr/bin/flex
YACC=/usr/bin/bison -d
LCFLAGS=

llgen: llgen.o gram2.tab.o scan2.yy.o avl3.o storage.o regexp.o lang.o
	$(CC) $(CFLAGS) llgen.o gram2.tab.o scan2.yy.o avl3.o storage.o regexp.o  lang.o -o llgen

lldump: lldump.c gram2.tab.o scan2.yy.o avl3.o storage.o regexp.o paramlist.o
	$(CC) $(CFLAGS) lldump.c gram2.tab.o scan2.yy.o avl3.o storage.o regexp.o paramlist.o -o lldump

llgen.o: llgen.c storage.h avl3.h llgen.h gram2.tab.c regexp.h

lang.o: lang.c lang.h llgen.h

storage.o: storage.c storage.h

regexp.o: regexp.c regexp.h

gram2.tab.o: gram2.tab.c llgen.h avl3.h

gram2.tab.c: gram2.y
	$(YACC) -d gram2.y

scan2.yy.o: scan2.yy.c gram2.tab.c llgen.h avl3.h
	$(CC) $(CFLAGS) $(LCFLAGS) -c scan2.yy.c

scan2.yy.c: scan2.l
	$(LEX) scan2.l
	mv lex.yy.c scan2.yy.c

avl3.o: avl3.c avl3.h

structgram: structgram.o storage.o avl3.o
	$(CC) $(CFLAGS) structgram.o storage.o avl3.o -o structgram

structgram.c: structgram.ll
	llgen structgram.ll

dexpr: dexpr.main.o dexpr.o gregjul.o
	$(CC) $(CFLAGS) dexpr.main.o dexpr.o gregjul.o -lm -o dexpr

gregjul.o: gregjul.c gregjul.h
	$(CC) $(CFLAGS) -c gregjul.c

dexpr.main.o: dexpr.main.c
	$(CC) $(CFLAGS) -c dexpr.main.c

dexpr.o: dexpr.c
	$(CC) $(CFLAGS) -c dexpr.c

dexpr.c: dexpr.ll
	llgen dexpr.ll
