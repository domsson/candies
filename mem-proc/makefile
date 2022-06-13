CFLAGS += -Wall -O3
PREFIX := /usr/local
BINDIR := $(PREFIX)/bin
NAME := mem-proc

all: bin/$(NAME)

bin/$(NAME): src/$(NAME).c src/candies.h
	mkdir -p bin
	$(CC) $(CFLAGS) -o bin/$(NAME) src/$(NAME).c

install: all
	mkdir -p $(BINDIR)
	cp bin/* $(BINDIR)
	chmod +x $(BINDIR)/$(NAME)

install-strip: install
	strip $(BINDIR)/$(NAME)

uninstall: 
	rm -f $(BINDIR)/$(NAME)

clean:
	rm -f bin/$(NAME)

.PHONY = all install install-strip uninstall clean
