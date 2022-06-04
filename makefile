all:
	gcc -Ofast bbchess.c -o bbchess
	x86_64-w64-mingw32-gcc -Ofast bbchess.c -o bbchess.exe

debug:
	gcc bbchess.c -o bbchess
	x86_64-w64-mingw32-gcc bbchess.c -o bbchess.exe