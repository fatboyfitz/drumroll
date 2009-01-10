#include <jack/jack.h>
//#include "jackmidi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct {
    jack_client_t* client;
    jack_port_t* output_port;
    volatile char state;
    int num_notes;
} gJack;


void update_jack_state(int note) {
    gJack.state = gJack.state | (1 << note);
}

/*
 * Process callback:
 * converts the pad information read into midi notes
 * and sends them to a midi input port via jack
 */
int process(jack_nframes_t nframes, void* arg)
{
    unsigned char* buffer;
    void* port_buffer;
    unsigned char midi_data[3];
    int pad_number = 0;

    port_buffer = jack_port_get_buffer(gJack.output_port, 1);

    jack_midi_clear_buffer(port_buffer, nframes);

    for (pad_number = 0; pad_number < gJack.num_notes; pad_number++) {
        
        if (gJack.state & (1 << pad_number)) {

            midi_data[0] = 0x9A;
            midi_data[1] = 0x24 + pad_number;
            midi_data[2] = 0x7F;

            buffer = (unsigned char*) jack_midi_event_reserve(port_buffer, 0, 3, nframes);

            if (!buffer) {
                fprintf(stderr, "ERROR: jack_midi_event_reserve failed\n");
                shutdown();
                exit(-1);
            }

            memcpy(buffer, midi_data, 3);
        }
    }

    gJack.state = 0; // clear since we've handled everything

    return 0;
}

/*
 * Shutdown callback:
 * jack has been shut down, so we need to shut down too
 */
void jack_shutdown_callback_jackmidi(void* arg) {
    // Do Nothing
}

/*
 * Error callback:
 * prints the error reported by jack to stderr
 */
void jack_error_callback_jackmidi(const char *msg) {
    fprintf(stderr, "ERROR: (jack) %s\n", msg);
}

int jack_init(int num_notes, char state)
{
    jack_set_error_function(jack_error_callback_jackmidi);

    // create a "jack client"
    if ((gJack.client = jack_client_new("drumroll")) == 0) {
        fprintf(stderr, "ERROR: cannot create a jack client\n");
        return 1;
    }

    // setup callback
    jack_set_process_callback(gJack.client, process, 0);

    jack_on_shutdown(gJack.client, jack_shutdown_callback_jackmidi, 0);

    // register the output midi port
    gJack.output_port = jack_port_register(gJack.client, "output", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

    gJack.num_notes = num_notes;
    gJack.state = state;

    // start processing
    if (jack_activate(gJack.client)) {
        fprintf(stderr, "ERROR: cannot activate client\n");
        return 2;
    }

    return 0;
}
