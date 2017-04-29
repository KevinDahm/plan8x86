CFLAGS += -Wall -nostdlib -ffreestanding
LDFLAGS += -nostdlib -ffreestanding

mazegame: mazegame.exe
	../elfconvert mazegame.exe
	mv mazegame.exe.converted mazegame

mazegame.exe: mazegame.o ece391syscall.o ece391support.o maze.o modex.o blocks.o input.o text.o
	gcc ${LDFLAGS} -o $@ $^

# tr: modex.c ${HEADERS} text.o
# 	gcc ${CFLAGS} -DTEXT_RESTORE_PROGRAM=1 -o tr modex.c text.o

%.o: %.c
	gcc ${CFLAGS} -c -o $@ $<

%.o: %.s
	gcc ${CFLAGS} -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

clean:: clear
	rm -f *.o *~ a.out

clear:
	rm -f mazegame mazegame.exe tr input

