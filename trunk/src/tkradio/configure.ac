AC_INIT([tkradio], [1.0], [rec@elf.org], [tkradio])

AM_INIT_AUTOMAKE([foreign -Wall])

## -------------------- ##
## Tool/Compilers setup ##
## -------------------- ##

# Look for an wish interpreter.
AC_CHECK_FILE([/usr/bin/wish],[have_wish=yes],[have_wish=no])

if test "x$have_wish" = xno; then
	AC_MSG_ERROR([
-----------------------------------
  The wish interpreter required to run tkradio.
-----------------------------------])
fi

# check for required tcl packages
# neither tcl-snack nor tcl-tls are required, they'll be used
# if they are available, otherwise ignored

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo \
"-----------------------------------------------------

${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix: '${prefix}'.
Now type 'make install' to install the tkradio script.

-----------------------------------------------------"

