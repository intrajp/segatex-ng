# Makefile.am - use automake to generate Makefile.in

bin_PROGRAMS = segatexd-agent

AM_CXFLAGS = $(INIT_CFLAGS)

segatexd_agent_SOURCES = main.c 
segatexd_agent_LDADD = $(INIT_LIBS) -lselinux -lpthread -laudit
