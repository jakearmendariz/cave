#
#Cave Programming Language
#Jake Armendariz
#Makefile-- creates an executable: cave
# contains a valgrind under check
# cleans
ADT_NAME = cave
SOURCES	 = lval.c	env.c	builtin.c	main.c	debug.c	mpc.c	
OBJECTS	 = lval.o	env.o	builtin.o	main.o 	debug.o	mpc.o
HEADERS	 = lisp.h	mpc.h
EXEBIN	 = cave
FLAGS	 = -std=c99 -Wall 

all: $(EXEBIN)

$(EXEBIN) : $(OBJECTS) $(HEADERS)
	gcc -ledit -o $(EXEBIN) $(OBJECTS)

$(OBJECTS) : $(SOURCES) $(HEADERS)
	gcc -c $(FLAGS) $(SOURCES)

clean :
	rm -f $(EXEBIN) $(OBJECTS)

check:
	valgrind --leak-check=full $(EXEBIN)
