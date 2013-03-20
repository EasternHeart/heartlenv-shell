
INCLUDES = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"libheartlenvpanel\" \
	-DLIBXFCE4PANEL_COMPILATION \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

lib_LTLIBRARIES = \
	libheartlenvpanel-1.0.la

libheartlenvpanel_public_built_sources = \
	libheartlenvpanel-enum-types.h

libheartlenvpanel_built_sources = \
	$(libheartlenvpanel_public_built_sources) \
	libheartlenvpanel-alias.h \
	libheartlenvpanel-aliasdef.c \
	libheartlenvpanel-enum-types.c \
	libheartlenvpanel-marshal.c \
	libheartlenvpanel-marshal.h

libheartlenvpanel_headers = \
	libheartlenvpanel.h \
	libheartlenvpanel-config.h \
	libheartlenvpanel-enums.h \
	xfce-arrow-button.h \
	xfce-hvbox.h \
	xfce-panel-convenience.h \
	xfce-panel-macros.h \
	xfce-panel-macros-46.h \
	xfce-panel-plugin.h \
	xfce-panel-plugin-provider.h \
	xfce-panel-image.h

libheartlenvpanel_includedir = \
	$(includedir)/heartlenv/libheartlenvpanel-$(LIBXFCE4PANEL_VERSION_API)/libheartlenvpanel

libheartlenvpanel_include_HEADERS = \
	$(libheartlenvpanel_public_built_sources) \
	$(libheartlenvpanel_headers)

libheartlenvpanel_1_0_la_SOURCES = \
	$(libheartlenvpanel_built_sources) \
	$(libheartlenvpanel_headers) \
	libheartlenvpanel-config.c \
	xfce-arrow-button.c \
	xfce-hvbox.c \
	xfce-panel-convenience.c \
	xfce-panel-plugin.c \
	xfce-panel-plugin-provider.c \
	xfce-panel-image.c

libheartlenvpanel_1_0_la_CFLAGS =	 \
	$(GTK_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(PLATFORM_CFLAGS)

libheartlenvpanel_1_0_la_LDFLAGS = \
	-no-undefined \
	-export-dynamic \
	-version-info $(LIBXFCE4PANEL_VERINFO) \
	-export-symbols-regex '^[^_].*' \
	$(PLATFORM_LDFLAGS)

libheartlenvpanel_1_0_la_LIBADD = \
	$(GTK_LIBS) \
	$(LIBXFCE4UTIL_LIBS) \
	-lm

#
# Pkg-config file
#
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libheartlenvpanel-$(LIBXFCE4PANEL_VERSION_API).pc

#
# Build sources
#
if MAINTAINER_MODE

libheartlenvpanel-marshal.h: libheartlenvpanel-marshal.list Makefile
	$(AM_V_GEN) glib-genmarshal --prefix=_libheartlenvpanel_marshal --internal --header $< > $@

libheartlenvpanel-marshal.c: libheartlenvpanel-marshal.list Makefile
	$(AM_V_GEN) echo "#include <libheartlenvpanel/libheartlenvpanel-marshal.h>" > $@ \
	&& glib-genmarshal --prefix=_libheartlenvpanel_marshal --body $< >> $@

libheartlenvpanel-alias.h: make-libheartlenvpanel-alias.pl libheartlenvpanel.symbols
	$(AM_V_GEN) $(PERL) $(srcdir)/make-libheartlenvpanel-alias.pl $(srcdir)/libheartlenvpanel.symbols > $@

libheartlenvpanel-aliasdef.c: make-libheartlenvpanel-alias.pl libheartlenvpanel.symbols
	$(AM_V_GEN) $(PERL) $(srcdir)/make-libheartlenvpanel-alias.pl -def < $(srcdir)/libheartlenvpanel.symbols > $@

libheartlenvpanel-enum-types.h: $(libheartlenvpanel_headers) Makefile
	$(AM_V_GEN) ( cd $(srcdir) && glib-mkenums \
		--fhead "#ifndef __LIBXFCE4PANEL_ENUM_TYPES_H__\n#define __LIBXFCE4PANEL_ENUM_TYPES_H__\n#include <glib-object.h>\nG_BEGIN_DECLS\n" \
		--fprod "/* enumerations from \"@filename@\" */\n" \
		--vhead "GType @enum_name@_get_type (void) G_GNUC_CONST;\n#define XFCE_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
		--ftail "G_END_DECLS\n\n#endif /* !__LIBXFCE4PANEL_ENUM_TYPES_H__ */" \
		$(libheartlenvpanel_headers) ) > $@

libheartlenvpanel-enum-types.c:  $(libheartlenvpanel_headers) Makefile
	$(AM_V_GEN) ( cd $(srcdir) && glib-mkenums \
		--fhead "#include <libheartlenvpanel/libheartlenvpanel-enums.h>\n#include <libheartlenvpanel/libheartlenvpanel-enum-types.h>\n#include <libheartlenvpanel/libheartlenvpanel-alias.h>" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n\tstatic GType type = 0;\n\tif (type == 0) {\n\tstatic const G@Type@Value values[] = {"\
		--vprod "\t{ @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "\t{ 0, NULL, NULL }\n\t};\n\ttype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n\treturn type;\n}\n" \
		--ftail "\n#define __LIBXFCE4PANEL_ENUM_TYPES_C__\n#include <libheartlenvpanel/libheartlenvpanel-aliasdef.c>\n" \
		$(libheartlenvpanel_headers) ) > $@

DISTCLEANFILES = \
	$(libheartlenvpanel_built_sources)

BUILT_SOURCES = \
	$(libheartlenvpanel_built_sources)

CLEANFILES = \
	actual-abi \
	expected-abi

endif

EXTRA_DIST = \
	abicheck.sh \
	libheartlenvpanel.symbols \
	libheartlenvpanel-alias.h \
	libheartlenvpanel-aliasdef.c \
	libheartlenvpanel-marshal.list \
	make-libheartlenvpanel-alias.pl

if HAVE_GNUC_VISIBILITY
TESTS =	abicheck.sh
endif

# required for gtk-doc
dist-hook: all

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
