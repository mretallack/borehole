
JSMN=jsmn

CFLAGS+=-I${JSMN}/ -g
LDFLAGS+=-lcurl -g

OBJFILES=fetch.o 

fetch: fetch.o 

clean:
	rm -f ${OBJFILES}
	rm -f fetch
	
	
