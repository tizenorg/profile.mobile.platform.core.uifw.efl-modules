# initialization
AC_INIT([efl-modules], [0.1], [])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_MACRO_DIR([m4])

# checks for programs
AC_PROG_CC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

# checks for libraries

# checks for header files
AC_HEADER_STDC

# checks for typedefs, structures, and compiler characteristics
AC_C_CONST

# checks for library functions

# checks for pkg-config
PKG_PROG_PKG_CONFIG

# gettext
m4_ifdef([AM_GNU_GETTEXT_VERSION], [
AM_GNU_GETTEXT_VERSION([0.18])
])

m4_ifdef([AM_GNU_GETTEXT], [
AM_GNU_GETTEXT([external])
po_makefile_in=po/Makefile.in
have_po="yes"
],[
have_po="no"
])
AC_SUBST([LTLIBINTL])

if test "x${POSUB}" = "x" ; then
   have_po="no"
fi

AM_CONDITIONAL([HAVE_PO], [test "x${have_po}" = "xyes"])

requirements="elementary, icu-i18n"

PKG_CHECK_MODULES(ELEMENTARY, [${requirements}])

# Check host system
ELEMENTARY_VERSION=`$PKG_CONFIG --modversion elementary | awk -F . '{printf("%s.%s.%s",$1,$2,$3);}'`
MODULE_ARCH="v-$ELEMENTARY_VERSION"
AC_SUBST(MODULE_ARCH)
AC_DEFINE_UNQUOTED(MODULE_ARCH, "$MODULE_ARCH", "Module architecture")

# Generate files
AC_CONFIG_FILES([
		 Makefile
		 $po_makefile_in
		 ])

AC_OUTPUT

