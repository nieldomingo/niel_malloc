CFLAGS = -g -std=gnu99

tests: test_malloc test_calloc test_realloc

test_malloc: niel_malloc.o
	gcc $(CFLAGS) -o tests/test_malloc -Isrc/ tests/test_malloc.c src/niel_malloc.o 

test_calloc: niel_malloc.o
	gcc $(CFLAGS) -o tests/test_calloc -Isrc/ tests/test_malloc.c src/niel_malloc.o 

test_realloc: niel_malloc.o
	gcc $(CFLAGS) -o tests/test_realloc -Isrc/ tests/test_malloc.c src/niel_malloc.o 

niel_malloc.o: 
	gcc -c $(CFLAGS) -o src/niel_malloc.o src/niel_malloc.c

