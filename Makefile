CXX = c++17
FLAGS = -g -O0

all:
	g++ -c -o ./build/main.o ./src/main.cpp -std=$(CXX) $(FLAGS)
	g++ -o sting ./build/*.o
