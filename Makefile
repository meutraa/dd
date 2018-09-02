srcdir = .

CC = gcc
INCS = -I/usr/local/include/deltachat -I${srcdir}/include -L/usr/local/lib
LIBCC = -ldeltachat -lpthread -lcurl -lssl
WARNINGS = -Wall -Wextra -Wvla -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wjump-misses-init -Wdouble-promotion -Wshadow -Wformat=2
CFLAGS += -std=gnu11 -O3 -pipe -fomit-frame-pointer ${WARNINGS} -pedantic ${INCS}
#CFLAGS = -std=gnu11 -ggdb -Wall -Wextra -pedantic ${INCS}
LDFLAGS += ${LIBCC}

all:
	${CC} ${CFLAGS} -o dd src/*.c ${LDFLAGS} 

install: all
	@cp {${srcdir},/usr/bin}/dd

uninstall:
	@rm /usr/lib/libdeltachat.so
	@rm /usr/bin/dd

clean:
	@rm ${srcdir}/dd
