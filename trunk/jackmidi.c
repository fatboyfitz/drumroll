#include <jack/jack.h>
//#include "jackmidi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct {
    jack_client_t* client;
    jack_port_t* output_port;
    volatile char state;
    int num_pads;
} gJack;


void set_jack_state(char new_state) {
    gJack.state = new_state;
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
    char data[3]; // midi data
    int pad_number = 0;

    port_buffer = jack_port_get_buffer(gJack.output_port, 1);

    jack_midi_clear_buffer(port_buffer, nframes);

    for (pad_number = 0; pad_number < gJack.num_pads; pad_number++) {
        
        if (gJack.state & (1 << pad_number)) {

            data[0] = 0x9A;
            data[1] = 0x24 + pad_number;
            data[2] = 0x7F;

            buffer = (unsigned char*) jack_midi_event_reserve(port_buffer, 0, 3, nframes);

            if (!buffer) {
                fprintf(stderr, "ERROR: jack_midi_event_reserve failed\n");
                shutdown();
                exit(-1);
            }

            memcpy(buffer, data, 3);
        }
    }

    gJack.state = 0; // clear since we've handled everything

    return 0;
}

/*
 * Shutdown callback:
 * jack has been shut down, so we need to shut down too
 */
void jack_shutdown_callback_jackdrum(void* arg) {
    shutdown();
    // this kind of shutdown is considered to be clean
    exit(0);
}

/*
 * Error callback:
 * prints the error reported by jack to stderr
 */
void jack_error_callback_jackdrum(const char *msg) {
    fprintf(stderr, "ERROR: (jack) %s\n", msg);
}

int jack_init(int num_pads, char state)
{
    // we set the error callback early so we can catch the errors :)
    jack_set_error_function(jack_error_callback_jackdrum);

    // create a "jack client"
    if ((gJack.client = jack_client_new("drumroll")) == 0) {
        fprintf(stderr, "ERROR: cannot create a jack client\n");
        return 1;
    }

    // set jack processing callback (where the work is done)
    jack_set_process_callback(gJack.client, process, 0);

    // set the shutdown handler
    jack_on_shutdown(gJack.client, jack_shutdown_callback_jackdrum, 0);

    // open (register) the output midi port
    gJack.output_port = jack_port_register(gJack.client, "output", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

    gJack.num_pads = num_pads;
    gJack.state = state;

    // finally, activate the client, which causes jack to start
    // calling the processing callback
    if (jack_activate(gJack.client)) {
        fprintf(stderr, "ERROR: cannot activate client\n");
        return 2;
    }

    // no errors
    return 0;
}
