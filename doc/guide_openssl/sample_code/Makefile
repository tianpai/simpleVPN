CC=gcc
CFLAGS=-I.
DEPS = ssl.h
OBJ = runner.o ssl.o
LIBS=-lssl -lcrypto

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

runner: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f *.o *~ core runner

