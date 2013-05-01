
AC_INIT([QtRadio Build], [hermes], [andrew att montefusco dott com], [QtRadio])

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

# If you need to create a library, USE libtool:
AC_PROG_LIBTOOL


#
# git repository parameters copied in GITSTR in order to display in "About" panel
#
GITSTR=`(git show -s --pretty=format:"%h!%s" 2>&1 | tr -d '"')`"!"`(git branch --no-color 2>&1  | sed -e '/^[[^*]]/d' -e 's/* \(.*\)/\1/')`
if { ! ( git show ) 1>/dev/null 2>&1 || test "$GITSTR" = "fatal"; } ;
   then GITSTR=`cat $srcdir/GITSTR`
   else echo $GITSTR>$srcdir/GITSTR
fi
AC_SUBST(GITSTR)


# QtRadio with Qt.
AT_WITH_QT([core gui network opengl])
AT_REQUIRE_QT_VERSION(4.0.0, AC_MSG_ERROR([Bad Qt: at least 4.x is required !]))

AC_MSG_RESULT([QT version: $QT_VERSION])


QTVM=`echo $QT_VERSION | sed 's/\.[[0-9]]*\.[[0-9]]*//g'`

AC_MSG_RESULT([QT version major: $QTVM])
if test "x${QTVM}" = x5; then
   AC_MSG_RESULT([QT 5 detected: adding QtWidgets lib])
   AT_WITH_QT([+widgets])
fi

AC_MSG_RESULT([QT_LIBS: $QT_LIBS])

AC_MSG_RESULT([QT_PATH: $QT_PATH])

AC_MSG_RESULT([QT_INCPATH: $QT_INCPATH])

AC_MSG_RESULT([QT_INSTALL_HEADERS: $QT_INSTALL_HEADERS])
AC_MSG_RESULT([QT_INSTALL_HEADERS_UP: $QT_INSTALL_HEADERS_UP])

AC_MSG_RESULT([QT_ADDITIONAL_INCLUDE_PATH: $QT_ADDITIONAL_INCLUDE_PATH])
AC_MSG_RESULT([QT_ADDITIONAL_LDFLAG: $QT_ADDITIONAL_LDFLAG])

#
# Checks for libraries.
# Check both libraries binary and header files
#

have_codec2=no
AC_SEARCH_LIBS([codec2_create], [codec2],[have_codec2=yes])

have_samplerate=no
AC_SEARCH_LIBS([src_new], [samplerate],[have_samplerate=yes])

have_ortp=no
AC_SEARCH_LIBS([ortp_init], [ortp],[have_ortp=yes])

if test "x${have_codec2}" = xyes; then
    AC_CHECK_HEADERS([codec2.h], [], [have_codec2=no])
fi

if test "x${have_codec2}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The codec2 library and header file required to build ghpsdr3.
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

if test "x${have_ortp}" = xyes; then
    AC_CHECK_HEADERS([ortp/ortp.h], [], [have_ortp=no])
fi

if test "x${have_ortp}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The ortp library and header file required to build ghpsdr3.
-----------------------------------])

fi



AC_CONFIG_FILES([
    Makefile
])

AC_OUTPUT
