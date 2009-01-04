bin_PROGRAMS = drumroll
drumroll_SOURCES = alsamidi.c drumroll.c sdlaudio.c usb_utils.c alsamidi.h sdlaudio.h usb_utils.h
drumroll_LDADD = @LIBOBJS@
EXTRA_DIST=samples

install-data-hook:
	cp -R samples $(pkgdatadir)
