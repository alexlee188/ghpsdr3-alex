AC_INIT([radio-backend], [1.0], [rec@elf.org], [radio-backend])

AM_INIT_AUTOMAKE([foreign -Wall])

## -------------------- ##
## Tool/Compilers setup ##
## -------------------- ##

# Look for an expect interpreter.
AC_CHECK_FILE([/usr/bin/expect],[have_expect=yes],[have_expect=no])
AC_PROG_INSTALL

if test "x${have_expect}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The expect interpreter required to run radio-backend.
-----------------------------------])

fi
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

#
# check the compiler option really working
#
corw=`gcc -Q --help=target -march=native`

echo \
"-----------------------------------------------------

${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix: '${prefix}'.
Now type 'make install'

 to install the radio-backend script.

-----------------------------------------------------"

