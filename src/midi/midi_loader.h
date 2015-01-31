#ifndef MIDI_LOADER_H
#define MIDI_LOADER_H

#include "fluid_list.h"
#include "fluidsynth_priv.h" 
#include "fluid_midi.h"
#include <sndfile.h>
#include <string.h>
#include <ladspa.h>
#include <dssi.h>


typedef struct event_table_t{
    snd_seq_event_t *events;
    size_t length;
    size_t last_nframe;
    size_t nframes_since_last;
} event_table_t;

typedef void(*read_midi_callback)(fluid_midi_event_t *event, size_t msecs_since_last, void *userdata);

typedef struct read_midi_ctx_t {
    fluid_player_t  *player;
    fluid_track_t   *track;
    read_midi_callback callback;
    void            *callback_userdata;
} read_midi_ctx_t;

void print_snd_seq_event(snd_seq_event_t *event);
void load_midi_file(char *filename, read_midi_callback callback, void *callback_userdata);
#endif
