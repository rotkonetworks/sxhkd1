OUT      = sxhkd
VERCMD  ?= git describe --tags 2> /dev/null
VERSION := $(shell $(VERCMD) || cat VERSION)

CC ?= cc
CLANG_VERSION := $(shell $(CC) --version 2>/dev/null | grep -q "clang" && echo 1 || echo 0)
GCC_VERSION := $(shell $(CC) --version 2>/dev/null | grep -q "gcc" && echo 1 || echo 0)

CPPFLAGS += -D_POSIX_C_SOURCE=200112L -DVERSION=\"$(VERSION)\" -D_FORTIFY_SOURCE=2
CFLAGS   += -std=c23 -pedantic -Wall -Wextra -O2 -march=native
CFLAGS   += -fstack-protector-strong

ifeq ($(CLANG_VERSION),1)
    CFLAGS += -flto=thin
    LDFLAGS += -flto=thin
endif

ifeq ($(GCC_VERSION),1)
    CFLAGS += -flto -fgraphite-identity
    LDFLAGS += -flto
endif

LDFLAGS  += -pie -Wl,-z,relro,-z,now
LDLIBS    = -lxcb -lxcb-keysyms -lxcb-xkb

PREFIX    ?= /usr/local
BINPREFIX ?= $(PREFIX)/bin
MANPREFIX ?= $(PREFIX)/share/man
DOCPREFIX ?= $(PREFIX)/share/doc/$(OUT)

all: $(OUT)

debug: CFLAGS += -O0 -g
debug: CPPFLAGS += -DDEBUG
debug: $(OUT)

analyze:
	@echo "Running clang static analyzer..."
	$(CC) --analyze -Xanalyzer -analyzer-output=text \
	      $(CPPFLAGS) $(CFLAGS) src/*.c

sanitize: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
sanitize: debug

VPATH = src
OBJ   =

include Sourcedeps

$(OBJ): Makefile

$(OUT): $(OBJ)

install:
	mkdir -p "$(DESTDIR)$(BINPREFIX)"
	cp -pf $(OUT) "$(DESTDIR)$(BINPREFIX)"
	mkdir -p "$(DESTDIR)$(MANPREFIX)"/man1
	cp -p doc/$(OUT).1 "$(DESTDIR)$(MANPREFIX)"/man1
	mkdir -p "$(DESTDIR)$(DOCPREFIX)"
	cp -pr examples "$(DESTDIR)$(DOCPREFIX)"/examples

uninstall:
	rm -f "$(DESTDIR)$(BINPREFIX)"/$(OUT)
	rm -f "$(DESTDIR)$(MANPREFIX)"/man1/$(OUT).1
	rm -rf "$(DESTDIR)$(DOCPREFIX)"

doc:
	a2x -v -d manpage -f manpage -a revnumber=$(VERSION) doc/$(OUT).1.asciidoc

clean:
	rm -f $(OBJ) $(OUT)

.PHONY: all debug analyze sanitize install uninstall doc clean
