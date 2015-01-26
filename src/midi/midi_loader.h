#ifndef MIDI_LOADER_H
#define MIDI_LOADER_H

#include "fluid_list.h"
#include "fluidsynth_priv.h" //is this the right idea? just need to load libs in the right order? yep
#include "fluid_midi.h"
#include <sndfile.h>
#include <string.h>
#include <ladspa.h>
#include <dssi.h>
//load all of those in the same way as below? or is the problem with how fluid_midi is accessing this file? yeah basically order issue


typedef struct event_table_t{
    snd_seq_event_t *events;
    size_t length;
    size_t last_nframe;
    size_t nframes_since_last;
} event_table_t;

//typedef void(*read_midi_callback)(event_table_t *event_table, void *userdata);
typedef void(*read_midi_callback)(fluid_midi_event_t *event, size_t msecs_since_last, void *userdata);



typedef struct read_midi_ctx_t {
    fluid_player_t  *player;
    fluid_track_t   *track;
    read_midi_callback callback;
    void            *callback_userdata;
} read_midi_ctx_t;


//void insert_event(event_table_t *event_table, snd_seq_event_t *event){
//void delete_event(event_table_t *event_table, snd_seq_event_t *event){
//int compare_events(snd_seq_event_t *event1, snd_seq_event_t *event2){
//void replace_events(event_table_t *event_table, snd_seq_event_t *event){
//like this?
//void convert_event_format(fluid_midi_event_t *from, snd_seq_event_t *to);
//void print_snd_seq_event(snd_seq_event_t *event);
//void print_event_table (event_table_t *event_table);
//int get_events(void *data, fluid_midi_event_t *event);

void print_snd_seq_event(snd_seq_event_t *event);
void load_midi_file(char *filename, read_midi_callback callback, void *callback_userdata);
#endif
