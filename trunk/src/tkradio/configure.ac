AC_INIT([tkradio], [1.0])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign -Wall])

## -------------------- ##
## Tool/Compilers setup ##
## -------------------- ##

# Look for an wish interpreter.
AC_CHECK_FILE([/usr/bin/wish8.6],[have_wish=yes],[have_wish=no])
AC_CHECK_FILE([/usr/bin/tclsh8.6],[have_tclsh=yes],[have_tclsh=no])

if test "x$have_wish" = xno -o "x$have_tclsh" = xno; then
	AC_MSG_ERROR([
-----------------------------------
  The wish and tclsh interpreters are required to run tkradio.
-----------------------------------])
fi

# need snit from tcllib, but cannot check

AC_CONFIG_FILES([Makefile])
AC_CONFIG_SUBDIRS([sdr])
AC_OUTPUT
