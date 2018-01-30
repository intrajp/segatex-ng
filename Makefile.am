# Makefile.am - use automake to generate Makefile.in

SUBDIRS = server agent

SEGATEXD_CONF_PATH = @SEGATEXD_CONF_PATH@
SEGATEXD_DATA_PATH = @SEGATEXD_DATA_PATH@
SEGATEXD_PIDFILE = @SEGATEXD_PIDFILE@

install-data-local: install-segatexd_conf install-segatexd_data install-data-hook
uninstall-local: uninstall-segatexd_conf uninstall-segatexd_data uninstall-segatexd_pid

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

# install a tcp send data file if it is not already there
install-segatexd_data:
	@if [ -f $(DESTDIR)$(SEGATEXD_DATA_PATH) ]; then \
            echo "Data file will be overritten."; \
            $(INSTALL) -m 644 $(srcdir)/segatexd_tcp_send_data.txt $(DESTDIR)$(SEGATEXD_DATA_PATH); \
	else \
            $(MKDIR_P) $(DESTDIR)$(SEGATEXD_DATA_PATH); \
            $(INSTALL) -m 644 $(srcdir)/segatexd_tcp_send_data.txt $(DESTDIR)$(SEGATEXD_DATA_PATH); \
	fi

uninstall-segatexd_data:
	-rm -rf $(DESTDIR)$(SEGATEXD_DATA_PATH)

uninstall-segatexd_pid:
	-rm -rf $(DESTDIR)$(SEGATEXD_PID_PATH)

# create directory for pidfile and data
install-data-hook:
	$(MKDIR_P) $(DESTDIR)/var/run/segatexd
