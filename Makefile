all:
	bison -d -o syntax.cpp syntax.y
	lex -o tokens.cpp tokens.l
	g++ -std=c++11 -o main syntax.cpp tokens.cpp main.cpp

clean:
	rm main syntax.cpp syntax.hpp tokens.cpp
