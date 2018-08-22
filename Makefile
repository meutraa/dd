srcdir = .

CC = clang
INCS = -I/usr/local/include/deltachat -I${srcdir}/include
LIBCC = -ldeltachat -lpthread -lcurl
CFLAGS += -std=gnu11 -march=skylake -O3 -pipe -fomit-frame-pointer -Wall -Wextra -pedantic ${INCS}
LDFLAGS += ${LIBCC}

all:
	${CC} ${CFLAGS} -o dd src/*.c ${LDFLAGS} 
