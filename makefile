sish: internal.o main.o
	g++ -std=c++11 internal.o main.o -o sish

internal.o: internal.h internal.cpp
	g++ -std=c++11 -c internal.cpp

main.o: internal.h main.cpp
	g++ -std=c++0x -c main.cpp