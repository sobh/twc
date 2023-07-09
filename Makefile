# twc - TinyWL compositor

#---- Configuration ------------------------------------------------------------

APP = twc
VERSION = 0.1

# paths
PREFIX = /usr/local

# Dependencies
DEPS="wayland-client wayland-server wlroots"

# includes and libs
INCS = `pkg-config --cflags ${DEPS}`
LIBS = `pkg-config --libs   ${DEPS}`

# Preprocessor Directives
WLR_CPPFLAGS = -DWLR_USE_UNSTABLE

# flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -DVERSION=\"${VERSION}\" ${WLR_CPPFLAGS}
#CFLAGS   = -g -std=c99 -pedantic -Wall -O0 ${INCS} ${CPPFLAGS}
CFLAGS   = -std=c99 -pedantic -Wall -Wno-deprecated-declarations -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# compiler and linker
CC = cc

#---- Recipes ------------------------------------------------------------------
SRC = main.c
OBJ = ${SRC:.c=.o}

run: all
	./${APP}

all: options ${APP}

options:
	@echo Build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo

.c.o:
	${CC} -c ${CFLAGS} $<

${APP}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

compile_flags.txt: Makefile
	@echo "Generating the 'compile_flags.txt' for 'clangd'..."
	@rm -f $@
	@for flag in ${CFLAGS}; do echo $$flag >> $@;done

clean:
	rm -f ${APP} ${OBJ} ${APP}-${VERSION}.tar.gz

dist: clean
	mkdir -p ${APP}-${VERSION}
	cp -R LICENSE Makefile README ${SRC} ${APP}-${VERSION}
	tar -cf ${APP}-${VERSION}.tar ${APP}-${VERSION}
	gzip ${APP}-${VERSION}.tar
	rm -rf ${APP}-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${APP} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${APP}

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${APP}

.PHONY: run all options clean dist install uninstall
