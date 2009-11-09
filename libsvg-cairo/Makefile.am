noinst_LIBRARIES = libsvg-cairo.a
noinst_HEADERS = svg-cairo.h

AM_CPPFLAGS =				\
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	-I$(top_srcdir)/libsvg		\
	$(GIBBON_CFLAGS)

libsvg_cairo_a_SOURCES = \
	svg_cairo.c \
	svg-cairo.h \
	svg-cairo-internal.h \
	svg_cairo_sprintf_alloc.c \
	svg_cairo_state.c
