CFLAGS = `pkg-config --cflags gtk+-2.0` -g
LIBS = `pkg-config --libs gtk+-2.0`
LDFLAGS = -lm
OBJ = ddb_splitter_test.o ddb_splitter.o ddb_splitter_size_mode.o

ddb_splitter: $(OBJ)
	$(CC) -o ddb_splitter_test $(OBJ) $(LIBS) $(LDFLAGS)

clean:
	$(RM) $(OBJ) ddb_splitter_test

all: ddb_splitter
