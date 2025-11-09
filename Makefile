
.PHONY: all run

all:
	g++ compiler.cpp -o compiler

run:
	g++ compiler.cpp -o compiler
	./compiler