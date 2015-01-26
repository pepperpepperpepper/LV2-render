#include "midi_loader.h"  

event_table_t *event_table;

//{{{ TO REMOVE...event table functions
void insert_event(event_table_t *event_table, snd_seq_event_t *event){
  //inserts an event into the event table
  event_table->events = realloc(event_table->events, (event_table->length + 1) * sizeof(snd_seq_event_t));
  memcpy(&event_table->events[event_table->length], event, sizeof(snd_seq_event_t));
  event_table->length += 1;
}

void delete_event(event_table_t *event_table, snd_seq_event_t *event){
  //deletes an event in the event table
  size_t i;
  for (i=0; i< event_table->length; i++){
    if(compare_events(&event_table->events[i], event)){ 
 printf("removed_event\n");
      memcpy(&event_table->events[i], &event_table->events[i+1], sizeof(snd_seq_event_t)*(event_table->length - i -1));
      event_table->events = realloc(event_table->events, event_table->length * sizeof(snd_seq_event_t));
      event_table->length--;
      i--;
    }
  }
}

void delete_note_off_events(event_table_t *event_table){
  //removes note_off events after they have happened from the event table
  size_t i;
  size_t i;
  for (i=0; i< event_table->length; i++){
    if(event_table->events[i].type == SND_SEQ_EVENT_NOTEOFF){ 
 printf("removed_note_off_event\n");
      memcpy(&event_table->events[i], &event_table->events[i+1], sizeof(snd_seq_event_t)*(event_table->length - i -1));
      event_table->events = realloc(event_table->events, event_table->length * sizeof(snd_seq_event_t));
      event_table->length--;
      i--;
    }
  }
}

int compare_events(snd_seq_event_t *event1, snd_seq_event_t *event2){
  //compares events in the event table
  return (
    (event1->data.note.note    == event2->data.note.note) && 
    (event1->data.note.channel == event2->data.note.channel)
  ) ? 1 : 0;
}


void replace_events(event_table_t *event_table, snd_seq_event_t *event){
  //replaces events in the event table
  size_t i;
  for (i=0; i< event_table->length; i++){
    if(compare_events(&event_table->events[i], event)){ 
 printf("replaced_event\n");
      memcpy(&event_table->events[i], event, sizeof(snd_seq_event_t)); 
    }
  }
}

void print_event_table (event_table_t *event_table){
  unsigned int i;
  for(i=0; i< event_table->length; i++){ 
    printf(" - %d: ", i + 1);
    print_snd_seq_event(&event_table->events[i]); 

  }
  printf("--\n");
}
//}}}

void convert_event_format(fluid_midi_event_t *from, snd_seq_event_t *to){
  memset(to, 0, sizeof(snd_seq_event_t));
//{{{ from->type
   switch(from->type){

    case NOTE_ON: 
      to->type = SND_SEQ_EVENT_NOTEON; 
      to->data.note.note = from->param1;
      to->data.note.velocity = from->param2;
      break;
    case NOTE_OFF: 
      to->type = SND_SEQ_EVENT_NOTEOFF; 
      to->data.note.note = from->param1;
      to->data.note.off_velocity = from->param2; 
      break; 
    /*case FLUID_NONE: to->type = SND_SEQ_EVENT_SYSTEM; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_RESULT; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_NOTE; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_KEYPRESS; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CONTROLLER; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PGMCHANGE; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CHANPRESS; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PITCHBEND; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CONTROL14; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_NONREGPARAM; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_REGPARAM; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SONGPOS; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SONGSEL; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_QFRAME; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_TIMESIGN; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_KEYSIGN; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_START; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CONTINUE; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_STOP; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SETPOS_TICK; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SETPOS_TIME; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_TEMPO; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CLOCK; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_TICK; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_QUEUE_SKEW; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SYNC_POS; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_TUNE_REQUEST; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_RESET; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SENSING; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_ECHO; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_OSS; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CLIENT_START; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CLIENT_EXIT; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_CLIENT_CHANGE; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PORT_START; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PORT_EXIT; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PORT_CHANGE; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PORT_SUBSCRIBED; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_PORT_UNSUBSCRIBED; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR0; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR1; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR2; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR3; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR4; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR5; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR6; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR7; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR8; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR9; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_SYSEX; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_BOUNCE; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR_VAR0; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR_VAR1; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR_VAR2; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR_VAR3; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_USR_VAR4; break;
    case FLUID_NONE: to->type = SND_SEQ_EVENT_NONE; break; 
*/
   }
//}}}
  //to->data.note.channel = from->channel; 
  to->data.note.channel = 0; // FIXME force channel
  to->time.tick = 0;
}

void print_snd_seq_event(snd_seq_event_t *event){
  char note_event[20];
  switch(event->type){

    case SND_SEQ_EVENT_NOTEON:
      strcpy(note_event,"NOTE_ON"); 
      break;
    case SND_SEQ_EVENT_NOTEOFF:
      strcpy(note_event,"NOTE_OFF");
      break;
    break;
  }
  printf("event_type: %s", note_event);
  printf("channel: %d ", event->data.note.channel);
  printf("note: %d ", event->data.note.note);
  printf("velocity: %d ", event->data.note.velocity);
  printf("tick: %d ", event->time.tick);
  printf("\n");
}


int get_events(void *data, fluid_midi_event_t *event){
  read_midi_ctx_t *ctx = (read_midi_ctx_t *)data;
  fluid_player_t *player = ctx->player;
  fluid_track_t *track = ctx->track;
  snd_seq_event_t seq_event; 

  size_t last_nframe = event_table->last_nframe;
  event_table->last_nframe = (player->deltatime * track->ticks) * 44100 / 1000; // FIXME 44100 to ctx->samplerate
  event_table->nframes_since_last = event_table->last_nframe - last_nframe;

  convert_event_format(event, &seq_event);
  
  read_midi_callback cb = ctx->callback;
  if(cb){
      cb(event_table, ctx->callback_userdata);
  }
  delete_note_off_events(event_table);  

  switch(event->type){ 
    case NOTE_ON:
      insert_event(event_table, &seq_event); 
      break;
    case NOTE_OFF:
      replace_events(event_table, &seq_event);     
      break;
    default:
      break;
  }
#define DEBUG_MIDI 0
  
  if(DEBUG_MIDI){
    printf("event table last nframe: %u\n", event_table->last_nframe);
    printf("run_synth(instancehandle, %u,\n", event_table->nframes_since_last);
    print_event_table(event_table);  
    printf(", %u)\n", event_table->length);
  }

}


void load_midi_file(char *filename, read_midi_callback callback, void *callback_userdata){ 
  int i;
  fluid_player_t *player;
  fluid_playlist_item playlist_item;
  read_midi_ctx_t ctx;

  event_table = malloc(sizeof (event_table_t));
  event_table->events = NULL;
  event_table->length = 0;
  event_table->last_nframe = 0; 
  event_table->nframes_since_last = 0;
  playlist_item.filename = filename;
  player = (fluid_player_t *)new_fluid_player();
  player->playback_callback = &get_events; 
  player->playback_userdata = (void *)&ctx;
  ctx.player = player;
  ctx.callback = callback;
  ctx.callback_userdata = callback_userdata;
  fluid_player_load(player, &playlist_item);

  for(i = 0; i < player->ntracks; i++){
     ctx.track = player->track[i];
     fluid_track_send_events(player->track[i], player->synth, player, (unsigned int)-1);
  }

  delete_fluid_player(player);
}
