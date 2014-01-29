#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([ghpsdr3-perseus], [1.0], [andrew@montefusco.com])

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([perseus_audio.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([build-aux])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL


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

# compiling `bandscope.c' with per-target flags requires `AM_PROG_CC_C_O' in `configure.ac'
AM_PROG_CC_C_O


# Checks for libraries.

# AC_CHECK_LIB([usb], [usb_init])

AC_CHECK_LIB([portaudio], [Pa_Initialize], [],
             [
              echo "-------------------------------------------------"
              echo ""
              echo "Error! You need to have portaudio library around."
              echo "Ubuntu/Debian: sudo apt-get install portaudio19-dev"
              echo ""
              echo "-------------------------------------------------"
              exit -1
             ])

AC_CHECK_LIB([pthread], [pthread_create], [],
             [
              echo "-------------------------------------------------"
              echo ""
              echo "Error! You need to have pthread library around."
              echo ""
              echo "-------------------------------------------------"
              exit -1
             ])

AC_CHECK_LIB([samplerate], [src_new], [],
             [
              echo "-------------------------------------------------"
              echo ""
              echo "Error! You need to have samplerate library around."
              echo "Ubuntu/Debian: sudo apt-get install libsamplerate0-dev"
              echo ""
              echo "-------------------------------------------------"
              exit -1
             ])

AC_CHECK_LIB([perseus-sdr], [perseus_init], [],
             [
              echo "-------------------------------------------------"
              echo ""
              echo "Error! You need to have perseus-sdr library around."
              echo "In order to build it, please follow instructions in http://code.google.com/p/libperseus-sdr/source/browse/trunk/README"
              echo ""
              echo "-------------------------------------------------"
              exit -1
             ])

AC_CHECK_LIB([boost_program_options], [main], [],
             [
              echo "-------------------------------------------------"
              echo ""
              echo "Error! You need to have boost library around."
              echo "Ubuntu/Debian: sudo apt-get install libboost-dev libboost-all-dev"
              echo ""
              echo "-------------------------------------------------"
              exit -1
             ])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

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

