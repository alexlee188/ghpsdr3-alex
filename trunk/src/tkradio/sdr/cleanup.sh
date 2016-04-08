#
# cleanup
#
# after autoreconf -i
rm -f -r aclocal.m4
rm -f -r autom4te.cache/
rm -f -r configure
rm -f -r install-sh*
rm -f -r Makefile.in
rm -f -r missing
# after configure, done by make distclean
rm -f -r config.log
rm -f -r config.status
rm -f -r Makefile
# after emacs
rm -f *~
