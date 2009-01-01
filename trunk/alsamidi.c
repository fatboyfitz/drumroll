#include "alsamidi.h"

bool setup_sequencer(AlsaMidi *seq)
{
	int rv;
	rv = snd_seq_open(&seq->handle, "default", SND_SEQ_OPEN_OUTPUT, 0);
	if(rv != 0) {
		fprintf(stderr, "Unable to open sequencer handle\n");
		return false;
	}

	snd_seq_set_client_name(seq->handle, "USB Piano");
	seq->port = snd_seq_create_simple_port(seq->handle, "USB Piano Output Port", SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	if(seq->port < 0) {
		fprintf(stderr, "Unable to create sequencer port\n");
		return false;
	}

	return true;
}


void send_event(unsigned int note, unsigned int key, bool pressed, int velocity, AlsaMidi *seq)
{
	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);
	snd_seq_ev_set_source(&ev, seq->port);
	snd_seq_ev_set_subs(&ev);
	snd_seq_ev_set_direct(&ev);

	snd_seq_ev_set_note(&ev, 0, note, velocity, 0);
	if(pressed) {
		ev.type = SND_SEQ_EVENT_NOTEON;
	} else {
		ev.type = SND_SEQ_EVENT_NOTEOFF;
	}

	int rv = snd_seq_event_output(seq->handle, &ev);
	if(rv < 0) {
		fprintf(stderr, "Unable to send event\n");
	}
	snd_seq_drain_output(seq->handle);
}


void free_sequencer(AlsaMidi* seq)
{
	if(seq->handle != NULL) {
		snd_seq_close(seq->handle);
	}

    if(seq->port > 0) {
		snd_seq_delete_simple_port(seq->handle, seq->port);
	}


}

