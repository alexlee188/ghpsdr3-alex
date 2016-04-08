## Process this file with automake to produce Makefile.in

SUBDIRS = sdr

bin_SCRIPTS = tkradio

# echo 'puts $auto_path' | tclsh | grep -s /usr/local/share/tcltk > /dev/null
# will return true if we're dealing with the debian packaging of tcltk
# in which case we run
# ./install-sh -d /usr/local/share/tcltk/sdr
# ./install-sh sdr/* /usr/local/share/tcltk/sdr
# otherwise we need to find the right place some other way
#
# CFLAGS=
# LIBS=
# pa_simple.so: pa_simple.c
#	gcc -fPIC -shared -o $@ $(CFLAGS) $< $(LIBS)
