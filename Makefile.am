# Makefile.am - use automake to generate Makefile.in

SUBDIRS = src

EXTRA_DIST = segatex-ng.conf

SEGATEX_NG_CONF_PATH = @SEGATEX_NG_CONF_PATH@

install-data-local: install-segatex_ng_conf
uninstall-local: uninstall-segatex_ng_conf

# install a default configuration file if it is not already there
install-segatex_ng_conf:
	@if [ -f $(DESTDIR)$(SEGATEX_NG_CONF_PATH) ]; then \
            echo "$(DESTDIR)$(SEGATEX_NG_CONF_PATH) already exists, install will not overwrite"; \
	else \
            $(mkinstalldirs) `dirname $(DESTDIR)$(SEGATEX_NG_CONF_PATH)`; \
            $(INSTALL) -m 600 $(srcdir)/segatex-ng.conf $(DESTDIR)$(SEGATEX_NG_CONF_PATH); \
	fi
uninstall-segatex_ng_conf:
	-rm -f $(DESTDIR)$(SEGATEX_NG_CONF_PATH)


