#include <stdbool.h>
#include <alsa/asoundlib.h>

typedef struct {
    snd_seq_t * handle;
    int port;
} AlsaMidi;

bool setup_sequencer(AlsaMidi *seq);
void send_event(unsigned int note, unsigned int key, int velocity, bool pressed, AlsaMidi *seq);
void free_sequencer(AlsaMidi* seq);
