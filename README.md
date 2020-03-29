# Lisp
Jake Armendariz

## What is Lisp
A basic programming language. It uses mcp library to parse text into usuable parts.  Creates an abstract syntax tree to divide up the input into numbers, variables, functions, conditionals and more.

## Note
Learning from this book: http://www.buildyourownlisp.com/
Did not write the mcp.h, mcp.c, taken from authors library: https://github.com/orangeduck/mpc

## Lisp can:
define variables
define arrays
conjoin arrays
add, subtract, divide etc with variables

### Lisp.h
Defines each of the structures and the functions for entire language
### lval.c
Lisp Value class: manipualtes each value, a single instance can be a number, error, symbol, function, syntax operation, or quote operation.
### builtin.c
lisp builtin functions: Handles operations for numbers, lists, and whatever other functions the program can handle
### env.c
Lisp Enviorment class: handles the enviorment the program operates in. Adding and handeling builtin functions and the scope of variables and classes.

## Use: 
compile: ! cc -std=c99 -Wall main.c lval.c builtin.c
mpc.c debug.c  -ledit -o run
<br>
run: ! ./run

