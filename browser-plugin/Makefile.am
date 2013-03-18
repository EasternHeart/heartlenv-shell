
mozillalibdir = $(BROWSER_PLUGIN_DIR)

mozillalib_LTLIBRARIES = libheartlenv-shell-browser-plugin.la

libheartlenv_shell_browser_plugin_la_LDFLAGS = -module -avoid-version -no-undefined

libheartlenv_shell_browser_plugin_la_LIBADD = 	\
	$(BROWSER_PLUGIN_LIBS)

libheartlenv_shell_browser_plugin_la_SOURCES = 	\
	browser-plugin.c \
	npapi/npapi.h \
	npapi/npfunctions.h \
	npapi/npruntime.h \
	npapi/nptypes.h

libheartlenv_shell_browser_plugin_la_CFLAGS = 	\
	$(BROWSER_PLUGIN_CFLAGS)		\
	-DG_DISABLE_DEPRECATED			\
	-DG_LOG_DOMAIN=\"GnomeShellBrowserPlugin\"
