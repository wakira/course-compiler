LLVMCONF = llvm-config
CPPFLAGS = `$(LLVMCONF) --cppflags` -std=c++11
LDFLAGS = `$(LLVMCONF) --ldflags` -std=c++11 -ldl -lz -lncurses -lboost_program_options -rdynamic -pthread
LIBS = `$(LLVMCONF) --libs` 

all: present.o codegen.o
	bison -d -o syntax.cpp syntax.y
	lex -o tokens.cpp tokens.l
	g++ $(LDFLAGS) $(CPPFLAGS) -o ml syntax.cpp tokens.cpp main.cpp present.o codegen.o $(LIBS)
codegen.o: codegen.cpp codegen.h
	g++ $(CPPFLAGS) -c codegen.cpp
present.o: present.h present.cpp
	g++ $(CPPFLAGS) -c present.cpp
clean:
	rm ml syntax.cpp syntax.hpp tokens.cpp present.o codegen.o
