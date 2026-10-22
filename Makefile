all:
	clang lexer.c parser.c list.c ast.c main.c `llvm-config --cflags --ldflags --system-libs --libs core` -o kal
