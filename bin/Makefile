all: dependencies edubreak

dependencies: ../source/dependencies.c ../source/dtree.c ../source/tree.c ../source/myalloc.c ../source/dis.c
	gcc -g -o dependencies ../source/dependencies.c ../source/dtree.c ../source/tree.c ../source/myalloc.c ../source/dis.c

edubreak: 
	gcc -g -o edubreak ../source/edubreak.c ../source/tree.c