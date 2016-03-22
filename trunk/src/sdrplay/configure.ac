AC_INIT([ghpsdr3-sdrplay], [1.0], [rec@elf.org], [sdrplay])

AC_CONFIG_MACRO_DIR([build-aux])

AM_INIT_AUTOMAKE([foreign -Wall])

## -------------------- ##
## Tool/Compilers setup ##
## -------------------- ##

# Look for a C++ compiler.
AC_PROG_CXX
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([usb], [usb_init])
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Check for SDRplay API
AC_SEARCH_LIBS([mir_sdr_Init], [mirsdrapi-rsp],[have_sdrplay=yes])
if test "x${have_sdrplay}" = xyes; then
    AC_CHECK_HEADERS([ mirsdrapi-rsp.h], [], [have_sdrplay=no])
fi
if test "x${have_sdrplay}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The mirsdrapi-rsp library and header file required to build sdrplay_server.
-----------------------------------])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([inet_ntoa memset socket])

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
Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'
Now type 'make @<:@<target>@:>@'

 where the optional <target> is:

    all           - build all binaries
    install       - install everything

Compiler options in use:

${corw}

-----------------------------------------------------"

