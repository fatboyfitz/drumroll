/*
   Copyright (C) 2009 Todd Kirby (ffmpeg.php@gmail.com)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
   */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <getopt.h>

#include "usb_drumkit.h"

#ifdef HAVE_LIBASOUND
#include "alsamidi.h"
#endif

#ifdef HAVE_LIBJACK
#include "jackmidi.h"
#endif

#ifdef HAVE_LIBSDL_MIXER
#include "sdlaudio.h"
#endif

#ifdef HAVE_LIBFAKEKEY
#include "fakekey.h"
#endif

static volatile sig_atomic_t fatal_error_in_progress = 0;

/* GLOBAL COMMANDLINE FLAGS */
static int nosound = false;
static int alsamidi = false;
static int autoconnect_hydrogen = false;
static int jackmidi = false;
static int verbose = false;
static int fakekeys = false;

#ifdef HAVE_LIBSDL_MIXER

static Sound sounds[USB_DRUMKIT_NUM_PADS];

static int load_sounds(Sound *sounds) {
    char filename[1024];

    unsigned int pad_num;

    for (pad_num = 0; pad_num < USB_DRUMKIT_NUM_PADS; pad_num++) {
        sprintf(filename, "%s/pad%d.wav", SAMPLESDIR, pad_num + 1);

        if (verbose) {
            fprintf(stdout, "Loading sound file '%s' into pad %d\n", filename, pad_num + 1);
        }

        if (access(filename, R_OK) < 0) {
            fprintf(stderr, "ERROR: Can't load sound file '%s'.\n Reason: %s\n", 
                    filename, strerror(errno));
            return 1;
        }

        sounds[pad_num] = sdlaudio_load_sound(filename, pad_num);

        if (sounds[pad_num] == NULL) {
            fprintf(stderr,"Could not load %s\n", filename);
            return 2;
        }
    }

    return 0;
}

static int free_sounds(Sound *sounds) {

    unsigned int pad_num;
    for (pad_num = 0; pad_num < USB_DRUMKIT_NUM_PADS; pad_num++) {
        sdlaudio_free_sound(sounds[pad_num]);
    }

    return 0;
}
#endif

void process_drum_event(unsigned int pad_num)
{
#ifdef HAVE_LIBFAKEKEY
    if (fakekeys) {
        if (verbose) {
            fprintf(stdout, "Sending key press event for pad %d\n",pad_num);
        }
        fakekey_send(pad_num);
    }
#endif

#ifdef HAVE_LIBSDL_MIXER
    if (!nosound) {
        sdlaudio_play_sound(sounds[pad_num]);
    }
#endif

#ifdef HAVE_LIBASOUND
    if (alsamidi) {
        alsamidi_send_event(36 + pad_num, ALSAMIDI_MAX_VELOCITY);
    }
#endif

#ifdef HAVE_LIBJACK
    if (jackmidi) {
        jackmidi_update_state(pad_num);
    }
#endif
}


static void print_usage(char * program_name)
{
    fprintf(stdout, "Usage: %s [OPTIONS]\n\n", program_name);
#ifdef HAVE_LIBASOUND
    fprintf(stdout, "  -a, --alsamidi               Open alsa MIDI port.\n");
    fprintf(stdout, "  -A, --autoconnect-hydrogen   Connect to Hydrogen if it's running (implies --alsamidi)\n");
#endif
#ifdef HAVE_LIBJACK
    fprintf(stdout, "  -j, --jackmidi               Register as JACK MIDI client\n"); 
#endif
#ifdef HAVE_LIBSDL_MIXER
    fprintf(stdout, "  -n, --nosound                Don't play direct sounds (use when doing MIDI)\n");
#endif
    fprintf(stdout, "  -k, --keys                   Send fake key-presses.\n");
    fprintf(stdout, "  -v, --verbose                Display more info.\n");
    fprintf(stdout, "  -V, --version                Display version info\n");
    fprintf(stdout, "  -h, --help                   Display this help text\n");
}

static void parse_options(int argc, char** argv)
{
    int opt_index = 0;
    char c;

    static const struct option long_options[] = {
        {"help", no_argument,       0, 'h'},
#ifdef HAVE_LIBASOUND
        {"alsamidi", no_argument,   0, 'a'},
        {"autoconnect-hydrogen", no_argument,   0, 'A'},
#endif
#ifdef HAVE_LIBJACK
        {"jackmidi", no_argument,   0, 'j'},
#endif
#ifdef HAVE_LIBSDL_MIXER
        {"nosound", no_argument,    0, 'n'},
#endif
        {"keys", no_argument,    0, 'k'},
        {"verbose", no_argument,    0, 'v'},
        {"version", no_argument,    0, 'V'},
        {0, 0, 0, 0}
    };


    while ((c = getopt_long(argc, argv, "AhjanvkV", long_options, &opt_index)) != -1) {

        if (c == 0) {
            c = long_options[opt_index].val;
        }

        switch(c) {
#ifdef HAVE_LIBASOUND
            case 'A':
                autoconnect_hydrogen = true;
                /* Fallthru */
            case 'a':
                alsamidi = true;
                break;
#endif
#ifdef HAVE_LIBJACK
            case 'j':
                jackmidi = true;
                break;
#endif
#ifdef HAVE_LIBSDL_MIXER
            case 'n':
                nosound = true;
                printf("Sound Disabled\n");
                break;
#endif
            case 'h':
                print_usage(argv[0]);
                exit(0);
                break;
            case 'k':
                fakekeys = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'V':
                fprintf(stdout, "%s\n", PACKAGE_STRING);
                exit(0);
                break;
            default:
                fprintf(stdout, "unrecognized option %c\n", c);
                exit(0);
                break;
        }
    }
}


static void cleanup()
{
    usb_drumkit_close();

#ifdef HAVE_LIBSDL_MIXER
    if (!nosound) {
        free_sounds(sounds);
        sdlaudio_close_audio();
    }
#endif

#ifdef HAVE_LIBASOUND
    if (alsamidi) {
        alsamidi_free_sequencer();
    }
#endif
}


void termination_handler(int sig)
{
    /* Since this handler is established for more than one kind of signal, 
       it might still get invoked recursively by delivery of some other kind
       of signal.  Use a static variable to keep track of that. */
    if (fatal_error_in_progress) {
        raise (sig);
    }
    fatal_error_in_progress = 1;

    cleanup();

    /* Now reraise the signal.  We reactivate the signal's
       default handling, which is to terminate the process.
       We could just call exit or abort,
       but reraising the signal sets the return status
       from the process correctly. */
    signal (sig, SIG_DFL);
    raise (sig);
}


int main(int argc, char** argv)
{

    /* Set signals so we can do orderly cleanup when user terminates us. */
    if (signal (SIGINT, termination_handler) == SIG_IGN)
        signal (SIGINT, SIG_IGN);
    if (signal (SIGHUP, termination_handler) == SIG_IGN)
        signal (SIGHUP, SIG_IGN);
    if (signal (SIGTERM, termination_handler) == SIG_IGN)
        signal (SIGTERM, SIG_IGN);


    parse_options(argc, argv);

    if (usb_drumkit_open()) {
        fprintf(stderr, "ERROR: usb drumkit initialization failed. Quitting\n");
        cleanup();
        exit(1);
    }

#ifdef HAVE_LIBSDL_MIXER
    if (!nosound) { 
        if (sdlaudio_init_audio() != 0) {
            fprintf(stderr, "ERROR: audio initialization failed. Quitting\n");
            cleanup();
            exit(2);
        }

        if (load_sounds(sounds)) {
            fprintf(stderr, "ERROR: loading sounds. Quitting.\n");
            cleanup();
            exit(3);
        }
    }
#endif

#ifdef HAVE_LIBASOUND
    if (alsamidi) {
        if (alsamidi_setup_sequencer("drumroll", "Output")) {
            fprintf(stderr, "ERROR: ALSA initialization failed. Quitting.\n");
            cleanup();
            exit(4);
        }

        if (autoconnect_hydrogen) {
            alsamidi_connect("drumroll", "Hydrogen");
        }
    }
#endif

#ifdef HAVE_LIBJACK
    if (jackmidi) {
        if (jackmidi_init(USB_DRUMKIT_NUM_PADS)) {
            fprintf(stderr, "ERROR: jack initialization failed. Quitting\n");
            cleanup();
            exit(5);
        }
    }
#endif

#ifdef HAVE_LIBFAKEKEY
    if (fakekeys) {
        if (fakekey_setup(USB_DRUMKIT_NUM_PADS)) {
            fprintf(stderr,"ERROR: fakekey failed to open display\n");
            cleanup();
            exit(6);
        }
    }
#endif

    if (usb_drumkit_process_events(process_drum_event)) {
        fprintf(stderr, "ERROR: processing drum events. Quitting\n");
        cleanup();
        exit(7);
    }

    cleanup();

    return 0;
}
