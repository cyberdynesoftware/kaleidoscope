all: libpass
	clang lexer.c parser.c list.c ast.c main.c `llvm-config --cflags --ldflags --system-libs --libs core` -lpass -L. -o kal

libpass: pass
	clang++ -shared -o libpass.so pass.o

pass:
	clang++ -c LLVMOptimizationPasses.cpp -o pass.o -fPIC
