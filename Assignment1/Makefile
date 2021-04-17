###############################################################################
# Makefile for assignment 1, Parallel and Distributed Computing 2020.
###############################################################################

CC = mpicc
CFLAGS = -std=c99 -g -O3
LIBS = -lm

BIN = stencilmpi_v0

all: $(BIN)

stencilmpi_v0: stencilmpi_v0.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
	
clean:
	$(RM) $(BIN)
