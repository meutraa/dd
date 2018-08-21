CC = gcc
INCS = -I/usr/local/include/deltachat
LIBS = -ldeltachat -lpthread -lcurl
CFLAGS += -march=skylake -O3 -pipe -fomit-frame-pointer -Wall -Wextra -pedantic ${INCS}
LDFLAGS += ${LIBS}

all:
	${CC} ${CFLAGS} -o dd network.c main.c ${LDFLAGS} 

install:
	mkdir -p ${HOME}/.config/systemd/user
	cp services/dd.service ${HOME}/.config/systemd/user/
