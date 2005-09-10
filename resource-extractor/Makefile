LDFLAGS=-lpng

all: resource-extractor

resource-extractor: resource-extractor.o
	${CC} ${LDFLAGS} -o resource-extractor resource-extractor.o

clean:
	rm resource-extractor.o resource-extractor
