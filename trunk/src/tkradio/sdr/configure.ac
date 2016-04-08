AC_INIT([sdr], [1.0])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign -Wall])

## -------------------- ##
## Tool/Compilers setup ##
## -------------------- ##

# Look for an wish interpreter.
AC_CHECK_FILE([/usr/bin/wish8.6],[have_wish=yes],[have_wish=no])
AC_CHECK_FILE([/usr/bin/tclsh8.6],[have_tclsh=yes],[have_tclsh=no])

if test "x$have_tclsh" = xno -o "x$have_wish" = xno; then
	AC_MSG_ERROR([
-----------------------------------
  The tclsh and wish interpreters are required.
  Install the tcl and tk packages.
-----------------------------------])
fi

# check for required packages
# need libpulse-dev to build pa_simple.so
AC_CHECK_HEADER([pulse/simple.h],[have_pulse_simple_h=yes],[have_pulse_simple_h=no])
if test "x$have_pulse_simple_h" = xno; then
   AC_MSG_ERROR([
-----------------------------------
  The pulse simple development library is required.	
  Install the libpulse-simple-dev package.
-----------------------------------])
fi

# need snit from tcllib, but cannot check
# need to decide where tcl packages get installed, but cannot check

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
