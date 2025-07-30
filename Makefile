gol: gol.c
	cc -O3 -o gol gol.c

gol.o: gol.c
	cc -DLIBGOLONLY -c -O3 -o gol.o gol.c
