CCOMPILER = clang
COMPILE_ARGS = -g -O2 -Wall -std=c11 -o a.out
SOURCES = srv_main.c srv_functions.c srv_utils.c srv_var.c
HEADERS = srv_error.h srv_header.h
a.out: ${SOURCES} ${HEADERS}
	${CCOMPILER} ${COMPILE_ARGS} ${SOURCES}
clean:
	rm a.out