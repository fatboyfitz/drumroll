#include <jack/jack.h>
#include "jackmidi.h"

struct OpaqueJack {
    jack_client_t* client = NULL;
    jack_port_t* output_port = NULL;
};

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

    port_buffer = jack_port_get_buffer(output_port, 1);

    jack_midi_clear_buffer(port_buffer, nframes);

    for (pad_number = 0; pad_number < 6; pad_number++) {
        if (pad_notes[pad_number]) {
            data[0] = 0x9A;
            data[1] = 0x24 + pad_number;
            data[2] = 0x7F;

            pad_notes[pad_number] = 0; // note is going to be played

            buffer = (unsigned char*) jack_midi_event_reserve(port_buffer, 0, 3, nframes);

            if (!buffer) {
                fprintf(stderr, "ERROR: jack_midi_event_reserve failed\n");
                shutdown();
                exit(-1);
            }

            memcpy(buffer, data, 3);
        }
    }

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

int jack_init() {

    // we set the error callback early so we can catch the errors :)
    jack_set_error_function(jack_error_callback_jackdrum);

    // create a "jack client"
    if ((client = jack_client_new("jackdrum")) == 0) {
        fprintf(stderr, "ERROR: cannot create a jack client\n");
        return -1;
    }

    // set jack processing callback (where the work is done)
    jack_set_process_callback(client, process, 0);

    // set the shutdown handler
    jack_on_shutdown(client, jack_shutdown_callback_jackdrum, 0);

    // open (register) the output midi port
    output_port = jack_port_register(client, "output", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

    // finally, activate the client, which causes jack to start
    // calling the processing callback
    if (jack_activate(client)) {
        fprintf(stderr, "ERROR: cannot activate client\n");
        return -1;
    }

    // no errors
    return 0;
}
