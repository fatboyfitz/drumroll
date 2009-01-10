bin_PROGRAMS = drumroll
drumroll_SOURCES = drumroll.c usb_drumkit.c usb_drumkit.h usb_utils.c usb_utils.h

if INCLUDE_JACK_SOURCES
    drumroll_SOURCES += jackmidi.c jackmidi.h
endif

if INCLUDE_ALSA_SOURCES
    drumroll_SOURCES += alsamidi.c alsamidi.h
endif

if INCLUDE_SDL_SOURCES
    drumroll_SOURCES += sdlaudio.c sdlaudio.h
endif

EXTRA_drumroll_SOURCES = alsamidi.c alsamidi.h sdlaudio.c sdlaudio.h jackmidi.c jackmidi.h

drumroll_LDADD = @LIBOBJS@


EXTRA_DIST=samples


install-data-hook:
	cp -R samples $(pkgdatadir)

AM_CPPFLAGS=-DSAMPLESDIR='"$(pkgdatadir)/samples"'

AM_CFLAGS=-Wall

# -Werror -pedantic
