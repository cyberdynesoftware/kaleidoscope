all: libpass
	clang lexer.c parser.c list.c ast.c main.c `llvm-config --cflags --ldflags --system-libs --libs core` -lstdc++ -lpass -L. -o kal

libpass: pass
	ar r libpass.a pass.o

pass:
	clang++ -c LLVMOptimizationPasses.cpp -o pass.o -fPIC
