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

#include "alsamidi.h"
#include <alsa/asoundlib.h>

struct OpaqueSeq {
    snd_seq_t * handle;
    int port;
};

struct OpaqueSeq* setup_sequencer()
{
    Seq seq = malloc(sizeof(struct OpaqueSeq));

	if (snd_seq_open(&seq->handle, "default", SND_SEQ_OPEN_OUTPUT, 0) != 0) {
		fprintf(stderr, "Unable to open sequencer handle\n");
        free(seq);
		return NULL;
	}

	snd_seq_set_client_name(seq->handle, "USB Roll-Up Drumkit");
	seq->port = snd_seq_create_simple_port(seq->handle, "USB Roll-Up Drumkit Output Port", SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	if(seq->port < 0) {
		fprintf(stderr, "Unable to create sequencer port\n");
        free(seq);
		return NULL;
	}

	return seq;
}


void send_event(unsigned int note, unsigned int key, int velocity, bool pressed, struct OpaqueSeq *seq)
{
	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);
	snd_seq_ev_set_source(&ev, seq->port);
	snd_seq_ev_set_subs(&ev);
	snd_seq_ev_set_direct(&ev);

    printf("send\n");
	snd_seq_ev_set_note(&ev, 0, note, velocity, 0);
    ev.type = SND_SEQ_EVENT_NOTE;

	if (snd_seq_event_output(seq->handle, &ev) < 0) {
		fprintf(stderr, "Unable to send event\n");
	}
	snd_seq_drain_output(seq->handle);
}


void free_sequencer(struct OpaqueSeq* seq)
{
	if(seq->handle != NULL) {
		snd_seq_close(seq->handle);
	}

    if(seq->port > 0) {
		snd_seq_delete_simple_port(seq->handle, seq->port);
	}

    free(seq);
}

