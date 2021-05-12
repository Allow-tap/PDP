CC = mpicc
CFLAGS = -std=c99 -g -O3
LIBS = -lm

BIN = qs_par

all: $(BIN)

qs_par: qs_par.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
clean:
	$(RM) $(BIN)