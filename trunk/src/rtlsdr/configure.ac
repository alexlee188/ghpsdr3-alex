AC_INIT([ghpsdr3-rtlsdr], [1.0], [andrew@montefusco.com], [rtlsdr])

#AC_CONFIG_AUX_DIR([build-aux])
#AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_MACRO_DIR([build-aux])

AM_INIT_AUTOMAKE([foreign -Wall])

## -------------------- ##
## Tool/Compilers setup ##
## -------------------- ##

# Look for a C++ compiler.
AC_LANG([C++])
AC_PROG_CXX


# Checks for libraries.

# AC_CHECK_LIB([usb], [usb_init])

AC_CHECK_LIB([portaudio], [Pa_Initialize])

AC_CHECK_LIB([pthread], [pthread_create])

AC_CHECK_LIB([samplerate], [src_new])

have_rtlsdr=no
#AC_CHECK_LIB([rtlsdr], [main])
AC_SEARCH_LIBS([rtlsdr_open], [rtlsdr],[have_rtlsdr=yes])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])


if test "x${have_rtlsdr}" = xyes; then
    AC_CHECK_HEADERS([ rtl-sdr.h], [], [have_rtlsdr=no])
fi

if test "x${have_rtlsdr}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The rtlsdr library and header file required to build rtlsdr_server.
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

echo \
"-----------------------------------------------------

${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix: '${prefix}'.
Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'
Now type 'make @<:@<target>@:>@'

 where the otpional <target> is:

    all           - build all binaries
    install       - install everything

-----------------------------------------------------"

