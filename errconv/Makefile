
ERRCONVOBJ = errconv.o errdef.o errcpp.o errjava.o
CFLAGS = -g
CPPFLAGS = -g
ALL = errconv 

all: $(ALL)

errconv: $(ERRCONVOBJ)
	$(CC) -o errconv $(ERRCONVOBJ)

clean:
	-rm -f $(ALL) *.o *~ core
