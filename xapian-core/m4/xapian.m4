# macro to get the libs/cxxflags for compiling with the xapian library
# serial 4

# AC_PROVIDE_IFELSE(MACRO-NAME, IF-PROVIDED, IF-NOT-PROVIDED)
# -----------------------------------------------------------
# If this macro is not defined by Autoconf, define it here.
m4_ifdef([AC_PROVIDE_IFELSE],
         [],
         [m4_define([AC_PROVIDE_IFELSE],
	         [m4_ifdef([AC_PROVIDE_$1],
		           [$2], [$3])])])

# XO_LIB_XAPIAN([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
# --------------------------------------------------------
# AC_SUBST-s XAPIAN_CXXFLAGS and XAPIAN_LIBS for use in Makefile.am
AC_DEFUN([XO_LIB_XAPIAN],
[
  AC_ARG_VAR(XAPIAN_CONFIG, [Location of xapian-config])
  AC_PATH_PROG(XAPIAN_CONFIG, xapian-config, [])
  if test -z "$XAPIAN_CONFIG"; then
    ifelse([$2], , :, [$2])
  else
    AC_MSG_CHECKING([$XAPIAN_CONFIG works])
    dnl run with exec to avoid leaking output on "real" bourne shells
    if (exec >&5 2>&5 ; $XAPIAN_CONFIG --libs --ltlibs --cxxflags; exit $?) then
      AC_MSG_RESULT(yes)
    else
      AC_MSG_ERROR([\`$XAPIAN_CONFIG --libs --ltlibs --cxxflags' doesn't work, aborting])
    fi

dnl If AC_PROG_LIBTOOL (or the deprecated older version AM_PROG_LIBTOOL)
dnl has already been expanded, enable libtool support now, otherwise add
dnl hooks to the end of AC_PROG_LIBTOOL and AM_PROG_LIBTOOL to enable it
dnl if either is expanded later.
    XAPIAN_CXXFLAGS="`$XAPIAN_CONFIG --cxxflags`"
    AC_PROVIDE_IFELSE([AC_PROG_LIBTOOL],
      [XAPIAN_LIBS="`$XAPIAN_CONFIG --ltlibs`"],
      [AC_PROVIDE_IFELSE([AM_PROG_LIBTOOL],
	[XAPIAN_LIBS="`$XAPIAN_CONFIG --ltlibs`"],
	[XAPIAN_LIBS="`$XAPIAN_CONFIG --libs`"
	define([AC_PROG_LIBTOOL], defn([AC_PROG_LIBTOOL])
	       [XAPIAN_LIBS="`$XAPIAN_CONFIG --ltlibs`"])
	define([AM_PROG_LIBTOOL], defn([AM_PROG_LIBTOOL])
	       [XAPIAN_LIBS="`$XAPIAN_CONFIG --ltlibs`"])])])
    ifelse([$1], , :, [$1])
  fi
  AC_SUBST(XAPIAN_CXXFLAGS)
  AC_SUBST(XAPIAN_LIBS)
])

dnl OM_PATH_XAPIAN([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Find paths to Xapian library
dnl Defines and AC_SUBST-s XAPIAN_CXXFLAGS and XAPIAN_LIBS
dnl This is deprecated - use XO_LIB_XAPIAN instead.
AC_DEFUN([OM_PATH_XAPIAN],
[dnl Get the cxxflags and libraries from the xapian-config script
AC_ARG_WITH(xapian-config,
[  --with-xapian-config    Location of xapian-config],
XAPIAN_CONFIG="$withval")

if test -z "$XAPIAN_CONFIG"; then
  XAPIAN_CONFIG=no
fi

if test no = "$XAPIAN_CONFIG"; then
  AC_PATH_PROG(XAPIAN_CONFIG, xapian-config, no)
fi

AC_MSG_CHECKING(for xapian)
if test no = "$XAPIAN_CONFIG"; then
  AC_MSG_RESULT(not found)
  ifelse([$2], , :, [$2])
else
  dnl run with exec to avoid leaking output on "real" bourne shells
  if (exec >&5 2>&5 ; $XAPIAN_CONFIG --libs --cxxflags; exit $?) then
    AC_MSG_RESULT(yes)
    XAPIAN_CXXFLAGS="`$XAPIAN_CONFIG --cxxflags`"
    XAPIAN_LIBS="`$XAPIAN_CONFIG --libs`"
    ifelse([$1], , :, [$1])
  else
    AC_MSG_ERROR([\`$XAPIAN_CONFIG --libs --cxxflags' doesn't work, aborting])
  fi
fi

AC_SUBST(XAPIAN_CXXFLAGS)
AC_SUBST(XAPIAN_LIBS)
])
