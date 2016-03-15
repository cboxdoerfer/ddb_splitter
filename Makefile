CFLAGS = `pkg-config --cflags gtk+-3.0` -g
LIBS = `pkg-config --libs gtk+-3.0`
LDFLAGS = -lm
OBJ = ddb_splitter_test.o ddb_splitter.o

ddb_splitter: $(OBJ)
	$(CC) -o ddb_splitter_test $(OBJ) $(LIBS) $(LDFLAGS)

clean:
	$(RM) $(OBJ) ddb_splitter_test

all: ddb_splitter
