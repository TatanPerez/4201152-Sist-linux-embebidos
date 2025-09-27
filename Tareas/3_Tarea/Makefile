CC=gcc
CFLAGS=-Wall -Wextra -O2 -std=c11
SRC=src/assignment-sensor.c
OUTDIR=build
BIN=$(OUTDIR)/assignment-sensor
PREFIX=/usr/local

all: $(BIN)

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(BIN): | $(OUTDIR)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

install: $(BIN)
	install -d $(PREFIX)/bin
	install -m 755 $(BIN) $(PREFIX)/bin/assignment-sensor

uninstall:
	rm -f $(PREFIX)/bin/assignment-sensor

clean:
	rm -rf $(OUTDIR)

.PHONY: all install uninstall clean
