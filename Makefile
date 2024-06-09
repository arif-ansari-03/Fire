all: compile link

compile:
	"C:\mingw64\bin\g++" -Isrc/include -c src/main.cpp

link:
	"C:\mingw64\bin\g++" main.o -o main -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system