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
#define JACK_MIDI

#ifdef ALSA_MIDI
#include "alsamidi.h"
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

AlsaMidi seq;

// GLOBAL COMMANDLINE FLAGS
int mute = false;
int midi = false;
int verbose = false;

/* A single drum pad */
typedef struct {
    Mix_Chunk *sound;
} Pad;


static const struct option long_options[] =
{
    {"help", no_argument, 0, 'h'},
    {"midi", no_argument, 0, 'm'},
    {"mute", no_argument, 0, 'x'},
    {"verbose", no_argument, 0, 'v'},
    {"version", no_argument, 0, 'V'},
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

                if (midi) {
                    send_event(pad_num, 48, 127, true, &seq);
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

    int i;
    for (i = 0; i < NUM_PADS; i++) {
        sprintf(filename, "%s/pad%d.wav", SAMPLES_DIR, i + 1);
        pads[i].sound = Mix_LoadWAV(filename);

        if (pads[i].sound == NULL) {
            fprintf(stderr,"Could not load %s\n", filename);
            return 1;
        }
    }

    return 0;
}


void print_usage(char * program_name)
{
    fprintf(stdout, "Usage: %s [OPTIONS]\n\n", program_name);
    fprintf(stdout, "  -a, --alsamidi\n");
    fprintf(stdout, "  -m, --mute\n");
}


void parse_options(int argc, char** argv)
{
    int opt_index = 0;
    char c;
    while((c = getopt_long(argc, argv, "hamvV", long_options, &opt_index)) != -1) {

        if(c == 0) {
            c = long_options[opt_index].val;
        }

        switch(c) {
            case 'a':
                midi = true;
                break;
            case 'm':
                mute = true;
                printf("muted\n");
                break;
            case 'h':
                print_usage(argv[0]);
                exit(0);
            case 'v':
                verbose = true;
                exit(0);
            case 'V':
                fprintf(stdout, "%s, %s\n", argv[0], DRUMROLL_VERSION);
                exit(0);
            default:
                fprintf(stdout, "unrecognized option %c\n", c);
                exit(0);
        }
    }
}


int main(int argc, char** argv)
{
    // run until kill or interrupt
    printf("drumroll starting...\n");

    parse_options(argc, argv);

    struct usb_device* usb_drumkit_device = NULL;  
    usb_dev_handle* drumkit_handle = NULL;

    Pad pads[NUM_PADS];

    usb_drumkit_device = get_usb_device(USB_VENDOR_ID_DREAM_CHEEKY, USB_DEVICE_ID_ROLL_UP_DRUMKIT);

    if (usb_drumkit_device  == NULL) {
        fprintf(stderr, "ERROR: couldn't find drumkit.\n");
        exit(2);
    }

    drumkit_handle = usb_open(usb_drumkit_device);

    if (drumkit_handle == NULL) {
        fprintf(stderr, "ERROR: opening drumkit\n");
        exit(3);
    }

    if (claim_usb_device(drumkit_handle, 0x00)) {
        fprintf(stderr, "ERROR: claiming drumkit\n");
        exit(3);
    }

    if (!mute) { 
        if (init_audio() != 0) {
            fprintf(stderr, "ERROR: audio initialization failed\n");
            exit(1);
        }

        load_sounds(pads);
    }

#ifdef ALSA_MIDI
    if (midi) {
        if (!setup_sequencer(&seq)) {
            free_sequencer(&seq);
            return 2;
        }
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

    printf("exiting drumroll...\n");
    exit(0);
    return 0;
}
