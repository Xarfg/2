CC=gcc

# uncomment to compile in 32bits mode (require gcc-*-multilib packages
# on Debian/Ubuntu)
#HOST32= -m32

CPPFLAGS= -I.
CFLAGS= $(HOST32) -Wall -Werror -std=c99 -g -fPIC -DMEMORY_SIZE=1024000
LDFLAGS= $(HOST32)


.PHONY: clean dist

all: memshell memtest
	./memtest

# dépendences des binaires
memshell: mem.o
memtest: mem.o

# nettoyage
clean::
	rm -f *.o memshell memtest libmalloc.so leak_test .*.deps

# dépendances des fichiers objects
$(patsubst %.c,%.o,$(wildcard *.c)): %.o: .%.deps

.%.deps: %.c
	$(CC) $(CPPFLAGS) -MM $< | sed -e 's/\(.*\).o: /.\1.deps \1.o: /' > $@

-include $(wildcard .*.deps)

# seconde partie du sujet
part2: leak_test libmalloc.so
	./leak_test
	$(MAKE) test_ls -R /

libmalloc.so: malloc_stub.o mem.o
	$(CC) -shared -Wl,-soname,$@ $^ -o $@

test_ls: libmalloc.so
	LD_PRELOAD=./libmalloc.so ls

test_ls_rec: libmalloc.so
	LD_PRELOAD=./libmalloc.so ls -R /

leak_test: malloc_stub.o mem.o

