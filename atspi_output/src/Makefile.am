MAINTAINERCLEANFILES = Makefile.in

pkgdir = $(libdir)/elementary/modules/atspi_output/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = mod.c

module_la_CPPFLAGS = \
-I. \
@ATSPI_OUTPUT_CFLAGS@

module_la_LIBADD =  \
@ATSPI_OUTPUT_LIBS@

module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static
