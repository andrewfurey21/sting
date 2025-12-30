CXX = c++17
FLAGS = -g -O0

all:
	@mkdir -p ./build
	# g++ -c -o ./build/scanner.o ./src/scanner.cpp -std=$(CXX) $(FLAGS)
	# g++ -c -o ./build/parser.o ./src/parser.cpp -std=$(CXX) $(FLAGS)
	# g++ -c -o ./build/value.o ./src/value.cpp -std=$(CXX) $(FLAGS)
	# g++ -c -o ./build/vmachine.o ./src/vmachine.cpp -std=$(CXX) $(FLAGS)
	# g++ -c -o ./build/interpreter.o ./src/interpreter.cpp -std=$(CXX) $(FLAGS)
	g++ -c -o ./build/object.o ./src/object.cpp -std=$(CXX) $(FLAGS)
	g++ -c -o ./build/main.o ./src/main.cpp -std=$(CXX) $(FLAGS)

	g++ -o sting ./build/*.o -fsanitize=address

clean:
	rm ./build/*.o
