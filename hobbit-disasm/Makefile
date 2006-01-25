LDFLAGS=-L/usr/local/lib -lelf
CFLAGS=-I/usr/local/include

all: hobbit-disasm

hobbit-disasm: hobbit-disasm.o
	${CC} -o hobbit-disasm hobbit-disasm.o ${LDFLAGS}

clean:
	rm hobbit-disasm hobbit-disasm.o
