LLVMCONF = llvm-config
CPPFLAGS = `$(LLVMCONF) --cppflags` -std=c++11 -g
LDFLAGS = `$(LLVMCONF) --ldflags` -std=c++11 -g -ldl -lz -lncurses -rdynamic
LIBS = `$(LLVMCONF) --libs` 

all: present.o codegen.o
	bison -d -o syntax.cpp syntax.y
	lex -o tokens.cpp tokens.l
	g++ $(LDFLAGS) $(LIBS) $(CPPFLAGS) -o main syntax.cpp tokens.cpp main.cpp present.o codegen.o
codegen.o: codegen.cpp codegen.h
	g++ $(CPPFLAGS) -c codegen.cpp
present.o: present.h present.cpp
	g++ $(CPPFLAGS) -c present.cpp
clean:
	rm main syntax.cpp syntax.hpp tokens.cpp present.o codegen.o
