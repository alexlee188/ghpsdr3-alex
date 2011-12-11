#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT([ghpsdr], [0.1], [andrew att montefusco dott com])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.9.6 -Wall -Werror dist-bzip2])

AC_PROG_CC
# Compiling sources with per-target flags requires AM_PROG_CC_C_O
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AM_PATH_GTK_2_0([2.10.0],,AC_MSG_ERROR([Gtk+ 2.10.0 or higher required.]))

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

# Checks for header files.

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



AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
        Makefile
])
AC_OUTPUT
