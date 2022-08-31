CFLAGS = -g -Wall -Wextra -std=gnu11
VFLAGS = --track-origins=yes --malloc-fill=0x40 --free-fill=0x23 \
         --leak-check=full --show-leak-kinds=all
BIN = ./load_example1/load_fs1 ./load_example2/load_fs2 ./load_example3/load_fs3 \
      ./create_example1/create_fs1 ./create_example2/create_fs2 ./create_example3/create_fs3


all: $(BIN)

./load_example1/load_fs1: load_fs1.o inode.o allocation.o
	gcc $(CFLAGS) $^ -o $@

./load_example2/load_fs2: load_fs2.o inode.o allocation.o
	gcc $(CFLAGS) $^ -o $@

./load_example3/load_fs3: load_fs3.o inode.o allocation.o
	gcc $(CFLAGS) $^ -o $@

load_fs1.o: load_example1/load_fs.c
	gcc $(CFLAGS) -I. -c $^ -o $@

load_fs2.o: load_example2/load_fs.c
	gcc $(CFLAGS) -I. -c $^ -o $@

load_fs3.o: load_example3/load_fs.c
	gcc $(CFLAGS) -I. -c $^ -o $@


./create_example1/create_fs1: create_fs1.o inode.o allocation.o
	gcc $(CFLAGS) $^ -o $@

./create_example2/create_fs2: create_fs2.o inode.o allocation.o
	gcc $(CFLAGS) $^ -o $@

./create_example3/create_fs3: create_fs3.o inode.o allocation.o
	gcc $(CFLAGS) $^ -o $@

create_fs1.o: create_example1/create_fs.c
	gcc $(CFLAGS) -I. -c $^ -o $@

create_fs2.o: create_example2/create_fs.c
	gcc $(CFLAGS) -I. -c $^ -o $@

create_fs3.o: create_example3/create_fs.c
	gcc $(CFLAGS) -I. -c $^ -o $@

%.o: %.c
	gcc $(CFLAGS) $^ -c


run_load1:
	cd load_example1 &&	./load_fs1

run_load2:
	cd load_example2 &&	./load_fs2

run_load3:
	cd load_example3 &&	./load_fs3

run_create1:
	cd create_example1 && ./create_fs1

run_create2:
	cd create_example2 && ./create_fs2

run_create3:
	cd create_example3 && ./create_fs3


valgrind_load1:
	cd load_example1 && valgrind $(VFLAGS) ./load_fs1

valgrind_load2:
	cd load_example2 && valgrind $(VFLAGS) ./load_fs2

valgrind_load3:
	cd load_example3 && valgrind $(VFLAGS) ./load_fs3

valgrind_create1:
	cd create_example1 && valgrind $(VFLAGS) ./create_fs1

valgrind_create2:
	cd create_example2 && valgrind $(VFLAGS) ./create_fs2

valgrind_create3:
	cd create_example3 && 	valgrind $(VFLAGS) ./create_fs3

clean:
	rm -rf *.o
	rm -f $(BIN)
