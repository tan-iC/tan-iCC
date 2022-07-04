CFLAGS=-std=c11	-g	-static

tan-iCC:	tan-iCC.c

test:	tan-iCC
	./test.sh

clean:
	rm	-f	tan-iCC	*.o	*~ tmp*

.PHONY:	test	clean
