ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src include/atres
EXTRA_DIST = autogen.sh

lib_LTLIBRARIES = libatres.la
libatres_la_SOURCES = 
libatres_la_LIBADD =  $(LIBOBJS) \
	src/libatres_src.la
libatres_la_LDFLAGS = -version-info @LT_VERSION@

