CFLAGS += -std=gnu11 -Wall -Wextra
LDFLAGS += -I/usr/local/include/deltachat -I./deps/ini -I./deps/flag -I./include -L/usr/local/lib
LDLIBS = -lssl -lcurl -lsqlite3 -lcrypto -lyaml -ldeltachat -lpthread 

all:
	$(CC) $(LDFLAGS) src/*.c deps/ini/*.c deps/flag/*.c $(LDLIBS) -o dd

install: all
	@cp dd /usr/bin/ddd

uninstall:
	@rm /usr/bin/ddd

clean:
	@rm ./dd
