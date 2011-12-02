#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([ghpsdr3], [2.1], [andrew att montefusco dott com])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Python checks for pylauncher
AM_PATH_PYTHON([2.5],, [:])

#
# Checking for build cpu
# and, if 64 bit, add some linking option
#
case $build_cpu in

    x86_64)
      AC_MSG_NOTICE([Platform: 64 bit])
      LDFLAGS="$LDFLAGS -L/lib64 -L/usr/lib64"
      export LDFLAGS
      AC_MSG_NOTICE([${LDFLAGS}])
    ;;


esac

#
# configure platform
# On a x86 PC with linux running $host is: i686-pc-linux-gnu
#
#
case $host in

  *darwin*)
    AC_MSG_NOTICE([Platform: MAC OSX])
    OSX_LDFLAGS=" -L/opt/local/lib"
    LDFLAGS="$LDFLAGS $OSX_LDFLAGS"
    export LDFLAGS
    ;;

  *linux*)
    AC_MSG_NOTICE([Platform LINUX])
    AC_MSG_NOTICE([Searching for CPU capability])
    MSSE3=`cat /proc/cpuinfo  | perl  -nle 'print "-msse3" if  m/ssse3|pni/'`
    MSSE3=`echo $MSSE3 | cut -d' ' -f1`  
    AC_SUBST(MSSE3)
    ;;

 *-*-*-*)
    AC_MSG_NOTICE([Platform unknown: $host])
    ;;

esac

# compiling `client.c' with per-target flags requires `AM_PROG_CC_C_O' in `configure.ac'
AM_PROG_CC_C_O

# library (in DttSP) used but `RANLIB' is undefined:  the usual way to define `RANLIB' is to add `AC_PROG_RANLIB'
AC_PROG_RANLIB

#
# Check for user options in order to selectively compile servers
#
AC_ARG_ENABLE([perseus],
              [  --enable-perseus  enable Perseus SDR server generation],
              [perseus=${enableval}], [perseus=no]
)
AC_ARG_ENABLE([usrp],
              [  --enable-usrp  enable USRP server generation],
              [usrp=${enableval}], [usrp=no]
)

#
# Checks for libraries.
# Check both libraries binary and header files
#

have_m=no
AC_SEARCH_LIBS([round], [m],[have_m=yes])

have_pthreads=no
AC_SEARCH_LIBS([pthread_create], [pthread],[have_pthreads=yes])

have_portaudio2=no
AC_SEARCH_LIBS([Pa_Initialize], [portaudio],[have_portaudio2=yes])

have_fftw3=no
AC_SEARCH_LIBS([fftwf_malloc], [fftw3f],[have_fftw3=yes])

have_codec2=no
AC_SEARCH_LIBS([codec2_create], [codec2],[have_codec2=yes])

have_pulse_simple=no
AC_SEARCH_LIBS([pa_simple_new], [pulse-simple],[have_pulse_simple=yes])

have_pulse=no
AC_SEARCH_LIBS([pa_strerror], [pulse],[have_pulse=yes])

have_usb=no
AC_SEARCH_LIBS([usb_init], [usb],[have_usb=yes])

have_jack=no
AC_SEARCH_LIBS([jack_client_open], [jack],[have_jack=yes])

have_config=no
AC_SEARCH_LIBS([config_init], [config],[have_config=yes])

have_usb10=no
AC_SEARCH_LIBS([libusb_init], [usb-1.0],[have_usb10=yes])

have_samplerate=no
AC_SEARCH_LIBS([src_new], [samplerate],[have_samplerate=yes])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h sys/timeb.h unistd.h])

if test "x${have_pthreads}" = xyes; then
    AC_CHECK_HEADERS([pthread.h], [], [have_pthreads=no])
fi

if test "x${have_pthreads}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The pthread library and header file required to build ghpsdr3.
-----------------------------------])

fi


if test "x${have_portaudio2}" = xyes; then
    AC_CHECK_HEADERS([portaudio.h], [], [have_portaudio2=no])
fi

if test "x${have_portaudio2}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The portaudio2 library and header file required to build ghpsdr3.
-----------------------------------])

fi


if test "x${have_fftw3}" = xyes; then
    AC_CHECK_HEADERS([fftw3.h], [], [have_fftw3=no])
fi

if test "x${have_fftw3}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The fftw3 library and header file required to build ghpsdr3.
-----------------------------------])

fi


if test "x${have_codec2}" = xyes; then
    AC_CHECK_HEADERS([codec2.h], [], [have_codec2=no])
fi

if test "x${have_codec2}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The codec2 library and header file required to build ghpsdr3.
-----------------------------------])

fi


if test "x${have_pulse_simple}" = xyes; then
    AC_CHECK_HEADERS([pulse/simple.h], [], [have_pulse_simple=no])
fi

if test "x${have_pulse_simple}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The pulse-simple library and header file required to build ghpsdr3.
-----------------------------------])

fi


if test "x${have_pulse}" = xyes; then
    AC_CHECK_HEADERS([pulse/error.h], [], [have_pulse=no])
fi

if test "x${have_pulse}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The pulse library and header file required to build ghpsdr3.
-----------------------------------])

fi


if test "x${have_usb}" = xyes; then
    AC_CHECK_HEADERS([usb.h], [], [have_usb=no])
fi

if test "x${have_usb}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The libusb-0.1 library and header file required to build ghpsdr3.
-----------------------------------])

fi

if test "x${have_jack}" = xyes; then
    AC_CHECK_HEADERS([jack/jack.h], [], [have_jack=no])
fi

if test "x${have_jack}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The libjack library and header file required to build ghpsdr3.
-----------------------------------])

fi

if test "x${have_config}" = xyes; then
    AC_CHECK_HEADERS([libconfig.h], [], [have_config=no])
fi

if test "x${have_config}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The libconfig8 library and header file required to build ghpsdr3.
-----------------------------------])

fi

if test "x${have_usb10}" = xyes; then
    AC_CHECK_HEADERS([libusb-1.0/libusb.h], [], [have_usb10=no])
fi

if test "x${have_usb10}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The libusb-1.0 library and header file required to build ghpsdr3.
-----------------------------------])

fi

if test "x${have_samplerate}" = xyes; then
    AC_CHECK_HEADERS([samplerate.h], [], [have_samplerate=no])
fi

if test "x${have_samplerate}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The samplerate library and header file required to build ghpsdr3.
-----------------------------------])

fi

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT32_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor ftime getcwd gethostbyname gettimeofday inet_ntoa memset pow select socket sqrt strrchr strstr strtol])



if test "x${perseus}" = xyes; then
    AC_MSG_NOTICE([
-----------------------------------
  Perseus SDR enabled.
-----------------------------------])  

    AC_CONFIG_SUBDIRS([trunk/src/perseus])
fi
AM_CONDITIONAL([MAKE_PERSEUS], [test "x${perseus}" = xyes])
AC_CONFIG_FILES([trunk/src/perseus/Makefile])


if test "x${usrp}" = xyes; then
    AC_MSG_NOTICE([
-----------------------------------
  USRP enabled.
-----------------------------------])  

    AC_CONFIG_SUBDIRS([trunk/src/usrp])
fi
AM_CONDITIONAL([MAKE_USRP], [test "x${usrp}" = xyes])
#
# Uncomment the following line when the usrp directory gets populated
#
#AC_CONFIG_FILES([trunk/src/usrp/Makefile])

AC_CONFIG_SUBDIRS([trunk/src/QtRadio])
AC_CONFIG_FILES([Makefile
                 trunk/src/softrock/Makefile
                 trunk/src/DttSP/Makefile
                 trunk/src/dspserver/Makefile
                 trunk/src/widget-server/Makefile
                 trunk/src/server/Makefile
                 trunk/src/pylauncher/Makefile])



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


build:         ${build}
build vendor:  ${build_vendor}
build os:      ${build_os} 
build cpu:     ${build_cpu}

host:          ${host}
host cpu:      ${host_cpu}
host vendor:   ${host_vendor}
host os:       ${host_os}


target:        ${target}
target cpu:    ${target_cpu}
target vendor: ${target_vendor}
target os:     ${target_os} 

-----------------------------------------------------"
