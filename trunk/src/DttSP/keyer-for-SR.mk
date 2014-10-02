CFLAGS = -O3 -I.
#CFLAGS = -g -O -I.
LIBS = ./libDttSP.a -ljack -lpthread -lfftw3f -lm

iambic-keyer-for-SR:	iambic-keyer-for-SR.o
	$(CC) -O3 -o iambic-keyer-for-SR iambic-keyer-for-SR.o $(LIBS)

keyboard-keyer-for-SR:	keyboard-keyer-for-SR.o
	$(CC) -o keyboard-keyer-for-SR keyboard-keyer-for-SR.o $(LIBS)

install:	iambic-keyer-for-SR keyboard-keyer-for-SR
	mv iambic-keyer-for-SR keyboard-keyer-for-SR ../bin

clean:
	/bin/rm -f *.o iambic-keyer-for-SR keyboard-keyer-for-SR\
			 ../bin/iambic-keyer-for-SR ../bin/keyboard-keyer-for-SR 

