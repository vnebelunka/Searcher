CC = g++
FLAGS = -std=c++17 -Wall -Wextra

all: Searcher

Searcher: main.o Searcher.o Searcher.h StreamSearch.o
	$(CC) $(FLAGS) StreamSearch.o Searcher.o main.o -o Searcher

main.o: main.cpp Searcher.h
	$(CC) $(FLAGS) main.cpp -c

Searcher.o: Searcher.h Searcher.cpp
	$(CC) $(FLAGS) Searcher.cpp -c

StreamSearch.o: StreamSearch.cpp Searcher.h
	$(CC) $(FLAGS) StreamSearch.cpp -c
clean:
	rm -f *.o
	rm -f Searcher
