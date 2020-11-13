CC=gcc

CFLAGS= -Wall -g -std=c11

LDLIBS= -lm -lreadline

ALL = mpsh

all : $(ALL)

mpsh : mpsh.c mpsh.h cmdmanager.o autocomplet.o variable.o utils/dictionaryTree.o int/echo.o int/cd.o int/exit.o int/alias.o int/unalias.o int/umask.o int/mkdir.o int/pwd.o int/type.o int/export.o int/history.o

autocomplet.o : autocomplet.c autocomplet.h cmdmanager.h variable.h

cmdmanager.o : cmdmanager.c cmdmanager.h mpsh.h variable.h int/echo.h int/cd.h int/exit.h int/alias.h int/unalias.h int/umask.h int/mkdir.h int/pwd.h int/type.h int/export.h autocomplet.h variable.h

variable.o : variable.c variable.h

dictionaryTree.o : utils/dictionaryTree.c utils/dictionaryTree.h 

echo.o : int/echo.c int/echo.h

cd.o : int/cd.c int/cd.h

exit.o : int/exit.c int/exit.h variable.h int/history.h

alias.o : int/alias.c int/alias.h cmdmanager.h variable.h

unalias.o : int/unalias.c int/unalias.h int/alias.h

umask.o : int/umask.c int/umask.h

mkdir.o : int/mkdir.c int/mkdir.h

pwd.o : int/pwd.c int/pwd.h

type.o : int/type.c int/type.h alias.h variable.h cmdmanager.h

export.o: int/export.c int/export.h

history.o: int/history.c int/history.h cmdmanager.h variable.h mpsh.h


cleanall :

	rm -rf *~ *.o */*~ */*.o $(ALL)
