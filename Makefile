all: present.o
	bison -d -o syntax.cpp syntax.y
	lex -o tokens.cpp tokens.l
	g++ -std=c++11 -g -o main syntax.cpp tokens.cpp main.cpp present.o
present.o: present.h present.cpp
	g++ -std=c++11 -g -c present.cpp
clean:
	rm main syntax.cpp syntax.hpp tokens.cpp present.o
