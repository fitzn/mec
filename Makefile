
CC=gcc
Flags=-Wall
Exe=mec.exe

all: $(Exe)

mec.exe: mec_test.c mec.o
	$(CC) $(Flags) $^ -o $@
mec.o: mec.c mec.h
	$(CC) $(Flags) -c $<

clean:
	rm -f $(Exe) *.o
