# Process this file with autoconf to produce a configure script.
#
# xfstt configure.ac
#
#	$Id$
#

AC_INIT([xfstt], [1.6])
AC_PREREQ([2.53])
AC_REVISION([$Revision$])
AC_CONFIG_SRCDIR([src/xfstt.cc])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign 1.5])
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_AWK
AC_PROG_YACC

# Checks for libraries.
AC_CHECK_LIB([m], [sqrt])

# Checks for header files.
AC_PATH_XTRA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS([fcntl.h limits.h netinet/in.h stddef.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

AC_CHECK_DECL([PF_INET6],
	      [AC_DEFINE([HAVE_IPV6], [1],
			 [The system has IPv6 support.])],
	      [], [#include <sys/socket.h>])
AC_CHECK_DECL([IPV6_V6ONLY],
	      [AC_DEFINE([HAVE_IPV6_V6ONLY], [1],
			 [The system has IPv6 v6 only support.])],
	      [], [#include <netinet/in.h>])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_STAT
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([gettimeofday pathconf memset munmap mkdir rmdir select socket sqrt])

# Checks for i18n
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.12.1])

AC_CONFIG_FILES([Makefile
		doc/Makefile
		po/Makefile.in
		libfstt/Makefile
                src/Makefile])
AC_OUTPUT

