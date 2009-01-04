bin_PROGRAMS = drumroll
drumroll_SOURCES = alsamidi.c drumroll.c sdlaudio.c usb_utils.c
drumroll_LDADD = @LIBOBJS@

install-data-hook:
	cp -R samples $(pkgdatadir)
