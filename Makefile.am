# Makefile.am - use automake to generate Makefile.in

SUBDIRS = src

SEGATEXD_CONF_PATH = @SEGATEXD_CONF_PATH@

install-data-local: install-segatexd_conf
uninstall-local: uninstall-segatexd_conf

# install a default configuration file if it is not already there
install-segatexd_conf:
	@if [ -f $(DESTDIR)$(SEGATEXD_CONF_PATH) ]; then \
            echo "$(DESTDIR)$(SEGATEXD_CONF_PATH) already exists, install will not overwrite"; \
	else \
            $(mkinstalldirs) `dirname $(DESTDIR)$(SEGATEXD_CONF_PATH)`; \
            $(INSTALL) -m 600 $(srcdir)/segatexd.conf $(DESTDIR)$(SEGATEXD_CONF_PATH); \
	fi
uninstall-segatexd_conf:
	-rm -f $(DESTDIR)$(SEGATEXD_CONF_PATH)


