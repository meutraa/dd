srcdir = .

CC = gcc
INCS = -I/usr/local/include/deltachat -I${srcdir}/deps/flag -I${srcdir}/include -L/usr/local/lib
LIBS = libssl libcurl sqlite3 libcrypto
LDFLAGS += -lyaml -ldeltachat -lpthread `pkg-config --libs ${LIBS}`
CLIBFLAGS = `pkg-config --cflags ${LIBS}`
WARNINGS = -Wall -Wextra
CFLAGS += -std=gnu11 -O3 -pipe -fomit-frame-pointer ${CLIBFLAGS} ${WARNINGS} -pedantic ${INCS}

all:
	${CC} ${CFLAGS} -o dd src/*.c deps/*/*.c ${LDFLAGS}

install: all
	@cp {${srcdir},/usr/bin}/dd

uninstall:
	@rm /usr/bin/dd

clean:
	@rm ${srcdir}/dd
