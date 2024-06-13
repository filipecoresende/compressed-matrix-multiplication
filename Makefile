CFLAGS= -Wall -Werror
LIB = lib

all: repair

repair: repair.c $(LIB)/hash.o $(LIB)/lista_ligada.o $(LIB)/pq_heap.o
	gcc $^ -o $@

#regra genérica
%.o: %.c %.h
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(LIB)/*.o repair decompressedString.txt




