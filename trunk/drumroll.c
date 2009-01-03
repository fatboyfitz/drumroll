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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <getopt.h>
#include <usb.h>

#include "usb_utils.h"

// TODO: Move these to ./configure script
#define SDL_SOUND
#define ALSA_MIDI
//#define JACK_MIDI

#ifdef ALSA_MIDI
#include "alsamidi.h"
#endif

#ifdef JACK_MIDI
#include "jackmidi.h"
#endif

#ifdef SDL_SOUND
#include <SDL.h>
#include "SDL_mixer.h"
#endif

#define DRUMROLL_VERSION "3.0" 

#define SAMPLES_DIR "/usr/share/drumroll/samples"

#define USB_VENDOR_ID_DREAM_CHEEKY 0x1941
#define USB_DEVICE_ID_ROLL_UP_DRUMKIT 0x8021

#define NUM_PADS 6

#define USB_INTERFACE_NUMBER 0x00

Seq seq;

// GLOBAL COMMANDLINE FLAGS
int mute = false;
int alsamidi = false;
int jackmidi = false;
int verbose = false;

/* A single drum pad */
typedef struct {
    Mix_Chunk *sound;
} Pad;


static const struct option long_options[] = {
    {"help", no_argument,       0, 'h'},
#ifdef ALSA_MIDI
    {"alsamidi", no_argument,   0, 'a'},
#endif
#ifdef JACK_MIDI
    {"jackmidi", no_argument,   0, 'j'},
#endif
    {"mute", no_argument,       0, 'm'},
    {"verbose", no_argument,    0, 'v'},
    {"version", no_argument,    0, 'V'},
    {0, 0, 0, 0}
};


static void close_audio()
{
    Mix_CloseAudio();
    SDL_Quit();
}


/*
 * Drumkit event loop
 */
static void start_processing_drum_events(usb_dev_handle* drumkit_handle, Pad *pads)
{
    char drum_state, last_drum_state = 0;

    // read pad status from device
    while (usb_bulk_read(drumkit_handle, 0x81, &drum_state, 1, 0) >= 0) {
        if (drum_state == last_drum_state) {
            continue;
        }

        int pad_num;
        for (pad_num = 0; pad_num < NUM_PADS; pad_num++) {
            if (drum_state & 1 << pad_num) {
                if (!mute) {
                    Mix_PlayChannel(pad_num, pads[pad_num].sound, 0);
                }

                if (alsamidi) {
                    send_event(36 + pad_num, 127, true, seq);
                }

                if (jackmidi) {
                    // NOTHING YET    
                }
            }
        }
        last_drum_state = drum_state;
    }

    fprintf(stderr, "ERROR: reading from usb device.\n Reason: %s\n", strerror(errno));
}


int init_audio()
{
    // Init SDL with Audio Support
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "WARNING: unable to initialize audio. Reason: %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(22050, AUDIO_S16, 2, 128) < 0) {
        fprintf(stderr, "WARNING: audio could not be setup for 11025 Hz 16-bit stereo.\nReason: %s\n", SDL_GetError());
        return 2;
    }

    return 0;
}


int load_sounds(Pad *pads) {
    char filename[256];

    int pad_num;
    for (pad_num = 0; pad_num < NUM_PADS; pad_num++) {
        sprintf(filename, "%s/pad%d.wav", SAMPLES_DIR, pad_num + 1);

        if (verbose) {
            fprintf(stdout, "Loading sound file '%s' into pad %d\n", filename, pad_num + 1);
        }

        pads[pad_num].sound = Mix_LoadWAV(filename);

        if (pads[pad_num].sound == NULL) {
            fprintf(stderr,"Could not load %s\n", filename);
            return 1;
        }
    }

    return 0;
}


void print_usage(char * program_name)
{
    fprintf(stdout, "Usage: %s [OPTIONS]\n\n", program_name);
#ifdef ALSA_MIDI
    fprintf(stdout, "  -a, --alsamidi\n");
#endif
#ifdef JACK_MIDI
    fprintf(stdout, "  -j, --jackmidi\n");
#endif
    fprintf(stdout, "  -m, --mute\n");
    fprintf(stdout, "  -v, --verbose\n");
    fprintf(stdout, "  -V, --version\n");
}


void parse_options(int argc, char** argv)
{
    int opt_index = 0;
    char c;
    while ((c = getopt_long(argc, argv, "hjamvV", long_options, &opt_index)) != -1) {

        if (c == 0) {
            c = long_options[opt_index].val;
        }

        switch(c) {
#ifdef ALSA_MIDI
            case 'a':
                alsamidi = true;
                break;
#endif
#ifdef JACK_MIDI
            case 'j':
                jackmidi = true;
                break;
#endif
            case 'm':
                mute = true;
                printf("muted\n");
                break;
            case 'h':
                print_usage(argv[0]);
                exit(0);
                break;
            case 'v':
                verbose = true;
                break;
            case 'V':
                fprintf(stdout, "Drumroll Version %s\n", DRUMROLL_VERSION);
                exit(0);
                break;
            default:
                fprintf(stdout, "unrecognized option %c\n", c);
                exit(0);
                break;
        }
    }
}


int main(int argc, char** argv)
{
    parse_options(argc, argv);

    struct usb_device* usb_drumkit_device = NULL;  
    usb_dev_handle* drumkit_handle = NULL;

    Pad pads[NUM_PADS];

    usb_drumkit_device = get_usb_device(USB_VENDOR_ID_DREAM_CHEEKY, USB_DEVICE_ID_ROLL_UP_DRUMKIT);

    if (usb_drumkit_device  == NULL) {
        fprintf(stderr, "ERROR: couldn't find drumkit.\n");
        exit(1);
    }

    drumkit_handle = usb_open(usb_drumkit_device);

    if (drumkit_handle == NULL) {
        fprintf(stderr, "ERROR: opening drumkit\n");
        exit(2);
    }

    if (claim_usb_device(drumkit_handle, 0x00)) {
        fprintf(stderr, "ERROR: claiming drumkit\n");
        exit(3);
    }

#ifdef SDL_SOUND
    if (!mute) { 
        if (init_audio() != 0) {
            fprintf(stderr, "ERROR: audio initialization failed\n");
            exit(4);
        }

        load_sounds(pads);
    }
#endif

#ifdef ALSA_MIDI
    if (alsamidi) {
        if ((seq = setup_sequencer("USB Roll-Up Drumkit", "Drumkit Port")) == NULL) {
            free_sequencer(seq);
            exit(5);
        }
    }
#endif

#ifdef JACK_MIDI
    if (jack_init()) {
        fprintf(stderr, "ERROR: jack initialization failed\n");
        exit(6);
    }
#endif

    start_processing_drum_events(drumkit_handle, pads);

    // NOT REACHED YET
    usb_release_and_close_device(drumkit_handle, USB_INTERFACE_NUMBER);

#ifdef SDL_SOUND
    if (!mute) {
        close_audio();
    }
#endif

#ifdef ALSA_MIDI
    if (alsamidi) {
        free_sequencer(seq);
    }
#endif

    exit(0);
    return 0;
}
