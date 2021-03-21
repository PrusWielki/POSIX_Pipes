FILES=main.c
TARGET=pipefork
FLAGS=-Wall -pedantic
${TARGET}: ${FILES}
	gcc ${FLAGS} -o ${TARGET} ${FILES}
.PHONY: clean

clean:
	-rm -f ${TARGET}
