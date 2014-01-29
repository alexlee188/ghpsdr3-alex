#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([ghpsdr3-usrp], [1.0], [BUG-REPORT-ADDRESS])

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([usrp_audio.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([build-aux])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
dnl # compiling `bandscope.c' with per-target flags requires `AM_PROG_CC_C_O' in `configure.ac'
dnl AM_PROG_CC_C_O

#
# Checking for build cpu
# and, if 64 bit, add some linking option
#
case $build_cpu in

    i686)
      AC_MSG_NOTICE([Platform: ix86 32 bit])
      # checks for platform dependent compiler optimizations
      AX_CHECK_COMPILE_FLAG(-ffast-math, SIMD_FLAGS="$SIMD_FLAGS -ffast-math", [])
      AX_CHECK_COMPILE_FLAG(-fopenmp, SIMD_FLAGS="$SIMD_FLAGS -fopenmp", [])
      SIMD_FLAGS="$SIMD_FLAGS  -march=native "
      AC_SUBST(SIMD_FLAGS)
    ;;

    x86_64)
      AC_MSG_NOTICE([Platform: ix86 64 bit])
      LDFLAGS="$LDFLAGS -L/lib64 -L/usr/lib64"
      export LDFLAGS
      AC_MSG_NOTICE([${LDFLAGS}])
      # checks for platform dependent compiler optimizations
      AX_CHECK_COMPILE_FLAG(-ffast-math, SIMD_FLAGS="$SIMD_FLAGS -ffast-math", [])
      AX_CHECK_COMPILE_FLAG(-fopenmp, SIMD_FLAGS="$SIMD_FLAGS -fopenmp", [])
      SIMD_FLAGS="$SIMD_FLAGS  -march=native "
      AC_SUBST(SIMD_FLAGS)
    ;;

    armv6l)
      AC_MSG_NOTICE([Platform: ARM 6])
      CFLAGS="-O3 -pipe -march=armv6 -mfpu=vfp -mfloat-abi=hard"
      CXXFLAGS="-O3 -pipe -march=armv6 -mfpu=vfp -mfloat-abi=hard"
      export CFLAGS
      export CXXFLAGS
      AC_MSG_NOTICE([${CFLAGS}])
      AC_MSG_NOTICE([${CXXFLAGS}])
    ;;

    armv7l)
      AC_MSG_NOTICE([Platform: ARM 7])
      CFLAGS="-O3 -pipe -march=armv7 -mfpu=neon -fno-tree-vectorize -fopenmp"
      CXXFLAGS="-O3 -pipe -march=armv7 -mfpu=neon -fno-tree-vectorize -fopenmp"
      export CFLAGS
      export CXXFLAGS
      AC_MSG_NOTICE([${CFLAGS}])
      AC_MSG_NOTICE([${CXXFLAGS}])
    ;;

esac

# Checks for libraries.

# we don't need to explicitly link the libusb because it is linked by uhd
# AC_CHECK_LIB([usb], [usb_init])

have_portaudio2=no
AC_SEARCH_LIBS([Pa_Initialize], [portaudio], [have_portaudio=yes])

have_pthreads=no
AC_SEARCH_LIBS([pthread_create], [pthread],[have_pthreads=yes])

have_samplerate=no
AC_SEARCH_LIBS([src_new], [samplerate],[have_samplerate=yes])

have_uhd=no
AC_LANG_PUSH(C++)
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([[#include <uhd/usrp/multi_usrp.hpp>]],[[uhd::device_addr_t hint;]])],
  [AC_MSG_NOTICE([uhd lib found])
   have_uhd=yes
   AC_SUBST(LIBS, "-luhd $LIBS")
  ],
  [AC_MSG_WARN([uhd lib not found])]
)
AC_LANG_POP([C++])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])


if test "x${have_portaudio}" = xyes; then
    AC_CHECK_HEADERS([portaudio.h], [], [have_portaudio=no])
fi

if test "x${have_portaudio}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The portaudio library and header file required to build usrp server.
-----------------------------------])

fi


if test "x${have_pthreads}" = xyes; then
    AC_CHECK_HEADERS([pthread.h], [], [have_pthreads=no])
fi

if test "x${have_pthreads}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The pthread library and header file required to build usrp server.
-----------------------------------])

fi


if test "x${have_samplerate}" = xyes; then
    AC_CHECK_HEADERS([samplerate.h], [], [have_samplerate=no])
fi

if test "x${have_samplerate}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The samplerate library and header file required to build usrp server.
-----------------------------------])

fi


if test "x${have_uhd}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The uhd library and header file required to build usrp server.
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

