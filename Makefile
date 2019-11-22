
JSMN=zserge-jsmn-6979f2e6ffa5/

CFLAGS+=-I${JSMN}/ -g
LDFLAGS+=-lcurl -g

OBJFILES=fetch.o ${JSMN}/jsmn.o

fetch: fetch.o ${JSMN}/jsmn.o

clean:
	rm -f ${OBJFILES}
	rm -f fetch
	
	
