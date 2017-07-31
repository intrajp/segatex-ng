# Makefile.am - use automake to generate Makefile.in

bin_PROGRAMS = segatexd

AM_CXFLAGS = $(INIT_CFLAGS)

segatexd_SOURCES = main.c 
segatexd_LDADD = $(INIT_LIBS) -lselinux -lpthread -laudit
