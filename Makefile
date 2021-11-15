IDIR =./include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR=./lib
SDIR=./src

TARGET=./bin/out

LIBS=-lSDL2 -lSDL2main -lm

_DEPS = simulation.h engine.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = simulation.o engine.o main.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS) -g

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: run test debug clean

run:
	$(TARGET)

test:
	make && make run

debug:
	make && gdb bin/out

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 