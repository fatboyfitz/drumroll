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

static snd_seq_t * seq_handle;
static int seq_port;

int alsamidi_setup_sequencer(const char *seq_client_name, const char* seq_port_name)
{
	if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) != 0) {
		fprintf(stderr, "Unable to open sequencer handle\n");
		return 1;
	}

	if (snd_seq_set_client_name(seq_handle, seq_client_name) < 0) {
        alsamidi_free_sequencer();		
		fprintf(stderr, "can't set client info\n");
		return 2;
	}

	seq_port = snd_seq_create_simple_port(seq_handle, seq_port_name, SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	if (seq_port < 0) {
        alsamidi_free_sequencer();
		fprintf(stderr, "Unable to create sequencer port\n");
		return 3;
	}

	return 0;
}


void alsamidi_send_event(unsigned int note, int velocity)
{
	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);
	snd_seq_ev_set_source(&ev, seq_port);
	snd_seq_ev_set_subs(&ev);
	snd_seq_ev_set_direct(&ev);

	snd_seq_ev_set_note(&ev, 0, note, velocity, 0);
    ev.type = SND_SEQ_EVENT_NOTEON;

	if (snd_seq_event_output(seq_handle, &ev) < 0) {
		fprintf(stderr, "Unable to send event\n");
	}

	snd_seq_drain_output(seq_handle);
}


void alsamidi_free_sequencer()
{
	if (seq_handle != NULL) {
		snd_seq_close(seq_handle);
	}

    if (seq_port > 0) {
		snd_seq_delete_simple_port(seq_handle, seq_port);
	}
}

static void error_handler(const char *file, int line, const char *function, int err, const char *fmt, ...)
{
	va_list arg;

	if (err == ENOENT)	/* Ignore those misleading "warnings" */
		return;
	va_start(arg, fmt);
	fprintf(stderr, "ALSA lib %s:%i:(%s) ", file, line, function);
	vfprintf(stderr, fmt, arg);
	if (err)
		fprintf(stderr, ": %s", snd_strerror(err));
	putc('\n', stderr);
	va_end(arg);
}


int alsamidi_connect(const char* src, const char* reciever)
{
	snd_seq_t *connector_seq;
	int client_id;
	snd_seq_port_subscribe_t *subs;
	snd_seq_addr_t sender, dest;
	
    snd_lib_error_set_handler(error_handler);

	if (snd_seq_open(&connector_seq, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
		fprintf(stderr, "can't open sequencer\n");
		return 1;
	} else {
        fprintf(stdout, "open sequencer\n");
    }
    
	if ((client_id = snd_seq_client_id(connector_seq)) < 0) {
		snd_seq_close(connector_seq);
		fprintf(stderr, "can't get client id\n");
		return 2;
	}

	/* set client info */
	if (snd_seq_set_client_name(connector_seq, "drumroll_midiconnector_client") < 0) {
		snd_seq_close(connector_seq);
		fprintf(stderr, "can't set client info\n");
		return 3;
	}

	/* set subscription */
	if (snd_seq_parse_address(connector_seq, &sender, src) < 0) {
		snd_seq_close(connector_seq);
		fprintf(stderr, "invalid sender address '%s' \n", src);
		return 4;
	}

	if (snd_seq_parse_address(connector_seq, &dest, reciever) < 0) {
		snd_seq_close(connector_seq);
		fprintf(stderr, "invalid destination address '%s'\n", reciever);
		return 5;
	}

	snd_seq_port_subscribe_alloca(&subs);
	snd_seq_port_subscribe_set_sender(subs, &sender);
	snd_seq_port_subscribe_set_dest(subs, &dest);

    if (snd_seq_get_port_subscription(connector_seq, subs) == 0) {
        snd_seq_close(connector_seq);
        fprintf(stderr, "Connection is already subscribed\n");
        return 6;
    }

    if (snd_seq_subscribe_port(connector_seq, subs) < 0) {
        snd_seq_close(connector_seq);
        fprintf(stderr, "Connection failed (%s)\n", snd_strerror(errno));
        return 7;
    }

	snd_seq_close(connector_seq);

    return 0;
}
