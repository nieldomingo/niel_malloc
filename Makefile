CFLAGS = -g -std=gnu99

tests: tests/test_malloc tests/test_calloc tests/test_realloc

tests/test_malloc: tests/test_malloc.c src/niel_malloc.o
	gcc $(CFLAGS) -o tests/test_malloc -Isrc/ tests/test_malloc.c src/niel_malloc.o 

tests/test_calloc: tests/test_calloc.c src/niel_malloc.o
	gcc $(CFLAGS) -o tests/test_calloc -Isrc/ tests/test_malloc.c src/niel_malloc.o 

tests/test_realloc: tests/test_realloc.c src/niel_malloc.o
	gcc $(CFLAGS) -o tests/test_realloc -Isrc/ tests/test_malloc.c src/niel_malloc.o 

src/niel_malloc.o: src/niel_malloc.h src/niel_malloc.c
	gcc -c $(CFLAGS) -o src/niel_malloc.o src/niel_malloc.c
