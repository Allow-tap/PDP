CC = mpicc
CFLAGS = -std=c99 -g -O3
LIBS = -lm

BIN = qs_par_v1

all: $(BIN)

qs_par_v1: qs_par_v1.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
clean:
	$(RM) $(BIN)