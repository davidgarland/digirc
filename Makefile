CC=cc
CFLAGS=
LDFLAGS=-Ilib/circa_core -Ilib/circa_txtbuf

default: build

build:
	idris --O2 src/backend.idr -o backend
	$(CC) $(CFLAGS) -c src/digirc.c $(LDFLAGS)
	$(CC) $(CFLAGS) -c src/irc.c $(LDFLAGS)
	$(CC) $(CFLAGS) *.o $(LDFLAGS)

clean:
	-@rm -f backend src/backend.ibc *.a *.o *.so *.out
