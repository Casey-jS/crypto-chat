CC = g++
CFLAGS = -lcrypto

all: server client clean

server: server.o crypt.o
	$(CC) -o $@ $^ $(CFLAGS)

client: client.o validation.o crypt.o
	$(CC) -o $@ $^ $(CFLAGS) -lpthread

server.o: server.cpp crypt.h
	$(CC) $(CFLAGS) -c $<

client.o: client.cpp validation.h crypt.h
	$(CC) $(CFLAGS) -c $<

validation.o: validation.cpp
	$(CC) -c $<

crypt.o: crypt.cpp
	$(CC) $(CFLAGS) -c $< 

clean:
	rm *.o
