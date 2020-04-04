# Cave- The Programming Language
Jake Armendariz

## What is Cave?
A basic programming language, based off of lisp, similar syntax to polish mathematical notation. It uses mcp library to parse text into usuable parts.  Creates an abstract syntax tree to divide up the input into numbers, variables, functions, conditionals and more.

## Note
Learning from this book: http://www.buildyourownlisp.com/
Did not write the mcp.h, mcp.c, taken from authors library: https://github.com/orangeduck/mpc

## Cave can:
Define variables, define arrays, conjoin arrays, add, subtract, divide etc with variables, for loops, iterate through array and working on more....

### Cave.h
Defines each of the structures and the functions for entire language
### cvalue.c
Cave Value function: manipualtes each value, a single instance can be a number, error, symbol, function, syntax operation, or quote operation.
### builtin.c
Cave's builtin functions: Handles operations for numbers, lists, and whatever other functions the program can handle
### env.c
Cave Enviorment functions: handles the enviorment the program operates in. Adding and handeling builtin functions and the scope of variables and classes.

## Use: 
compile (on mac/windows): ! make
<br>
compile (on linux):       ! cc -std=c99 -Wall mpc.c cvalue.c builtin.c env.c debug.c  main.c -lreadline -o cave
<br>
run: ! ./cave

