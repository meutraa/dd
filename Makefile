srcdir = .
deltachatdir = ${srcdir}/deltachat-core-0.20

CC = gcc
INCS = -I${deltachatdir}/include -I${srcdir}/include -L${deltachatdir}/lib
LIBCC = -ldeltachat -lpthread -lcurl -lssl
CFLAGS += -std=gnu11 -march=skylake -O3 -pipe -fomit-frame-pointer -Wall -Wextra -pedantic ${INCS}
#CFLAGS = -std=gnu11 -ggdb -Wall -Wextra -pedantic ${INCS}
LDFLAGS += ${LIBCC}

all:
	${CC} ${CFLAGS} -o dd src/*.c ${LDFLAGS} 
