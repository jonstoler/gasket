CFLAGS = -llua -lX11 -lcairo -I/usr/include/cairo

gasket: gasket.c

clean:
	rm gasket
