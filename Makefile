CC=cc
CFLAGS=
LDFLAGS=-Ilib/circa_core -Ilib/circa_txtbuf

default: build

build:
	$(CC) $(CFLAGS) -c src/digirc.c $(LDFLAGS)
	$(CC) $(CFLAGS) -c src/irc.c $(LDFLAGS)
	$(CC) $(CFLAGS) *.o $(LDFLAGS)

clean:
	-@rm -f backend *.a *.o *.so *.out
