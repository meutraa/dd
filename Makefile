srcdir = .

#linux-vdso.so.1 (0x00007ffc51fba000)
#libdeltachat.so => /usr/lib64/libdeltachat.so (0x00007eff6c549000)		DONE
#libpthread.so.0 => /lib64/libpthread.so.0 (0x00007eff6c527000)
#libssl.so.1.0.0 => /usr/lib64/libssl.so.1.0.0 (0x00007eff6c4b3000)		DONE
#libcurl.so.4 => /usr/lib64/libcurl.so.4 (0x00007eff6c438000)			DONE
#libsqlite3.so.0 => /usr/lib64/libsqlite3.so.0 (0x00007eff6c2d7000)		DONE
#libcrypto.so.1.0.0 => /usr/lib64/libcrypto.so.1.0.0 (0x00007eff6c083000)	DONE
#libc.so.6 => /lib64/libc.so.6 (0x00007eff6beb5000)
#libz.so.1 => /lib64/libz.so.1 (0x00007eff6be97000)
#libsasl2.so.3 => /usr/lib64/libsasl2.so.3 (0x00007eff6bc78000)			DONE
#/lib64/ld-linux-x86-64.so.2 (0x00007eff6c679000)
#libm.so.6 => /lib64/libm.so.6 (0x00007eff6baeb000)
#libdl.so.2 => /lib64/libdl.so.2 (0x00007eff6bae5000)


CC = clang
INCS = -I/usr/local/include/deltachat -I${srcdir}/include -L/usr/local/lib
LIBS = libssl libcurl sqlite3 libcrypto
LDFLAGS += -ldeltachat -lpthread `pkg-config --libs ${LIBS}`
CLIBFLAGS = `pkg-config --cflags ${LIBS}`
WARNINGS = -Wall -Wextra
CFLAGS += -std=gnu11 -O3 -pipe -fomit-frame-pointer ${CLIBFLAGS} ${WARNINGS} -pedantic ${INCS}

all:
	${CC} ${CFLAGS} -o dd src/*.c ${LDFLAGS} 

install: all
	@cp {${srcdir},/usr/bin}/dd

uninstall:
	@rm /usr/lib/libdeltachat.so
	@rm /usr/bin/dd

clean:
	@rm ${srcdir}/dd
