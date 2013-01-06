GTK_INCLUDES=-I/usr/include/gtk-3.0 -I/usr/lib/gtk-3.0/include\
         -I/usr/include/atk-1.0 -I/usr/include/cairo\
         -I/usr/include/pango-1.0 -I/usr/include/glib-2.0\
         -I/usr/lib/i386-linux-gnu/glib-2.0/include -I/usr/include/pixman-1\
         -I/usr/include/freetype2 -I/usr/include/libpng12\
	 -I/usr/include/gdk-pixbuf-2.0/

GTK_LIBS=-lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lpangoft2-1.0 -lgdk-pixbuf-2.0\
         -lpangocairo-1.0 -lgio-2.0 -lcairo -lpango-1.0 -lfreetype -lz\
         -lfontconfig -lgobject-2.0 -lgmodule-2.0 -lglib-2.0

all:
	gcc -o ../../bin/monitor -g $(GTK_INCLUDES) -lpthread -lfftw3f -lm $(GTK_LIBS) *.c
