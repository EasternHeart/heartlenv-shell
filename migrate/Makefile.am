
INCLUDES = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"heartlenv-panel-migrate\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	-DDEFAULT_CONFIG=\"$(sysconfdir)/xdg/heartlenv/panel/default.xml\" \
	$(PLATFORM_CPPFLAGS)

confdir = $(sysconfdir)/xdg/heartlenv/panel
conf_DATA = \
	default.xml

migratedir = $(HELPER_PATH_PREFIX)/heartlenv/panel

migrate_PROGRAMS = \
	migrate

migrate_SOURCES = \
	main.c \
	migrate-46.c \
	migrate-46.h \
	migrate-default.c \
	migrate-default.h

migrate_CFLAGS = \
	$(GTK_CFLAGS) \
	$(XFCONF_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(PLATFORM_CFLAGS)

migrate_LDFLAGS = \
	-no-undefined \
	$(PLATFORM_LDFLAGS)

migrate_LDADD = \
	$(top_builddir)/libheartlenvpanel/libheartlenvpanel-$(LIBXFCE4PANEL_VERSION_API).la \
	$(GTK_LIBS) \
	$(XFCONF_LIBS) \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS)

migrate_DEPENDENCIES = \
	$(top_builddir)/libheartlenvpanel/libheartlenvpanel-$(LIBXFCE4PANEL_VERSION_API).la

EXTRA_DIST = \
	default.xml

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
