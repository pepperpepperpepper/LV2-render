#include "midi_loader.h"  
#define DEBUG 0

//static int event_count = 0;
size_t last_msec = 0; 
size_t nmsecs_since_last = 0;



void print_event(fluid_midi_event_t *event, size_t current_msec){
//  {{{ DESCRIPTION
//  fluid_midi_event_t* next; /* Link to next event */
//  void *paramptr;           /* Pointer parameter (for SYSEX data), size is stored to param1, param2 indicates if pointer should be freed (dynamic if TRUE) */
//  unsigned int dtime;       /* Delay (ticks) between this and previous event. midi tracks. */
//  unsigned int param1;      /* First parameter */
//  unsigned int param2;      /* Second parameter */
//  unsigned char type;       /* MIDI event type */
//  unsigned char channel;    /* MIDI channel */
//}}}
  if(DEBUG){
    printf("dtime:%u ", event->dtime);
    printf("param1:%u ", event->param1);
    printf("param2: %u ", event->param2);
    printf("type: %x ", event->type);
    printf("channel: %u ", event->channel);
    printf("nframe: %u ", current_msec);
    puts("\n");
  }
}

int get_events(void *data, fluid_midi_event_t *event){
  //this function is called every time a new event comes in
  read_midi_ctx_t *ctx = (read_midi_ctx_t *)data;
  fluid_player_t *player = ctx->player;
  fluid_track_t *track = ctx->track;
  read_midi_callback cb = ctx->callback; 
  size_t current_msec;

  current_msec = (player->deltatime * track->ticks);
  nmsecs_since_last = current_msec - last_msec; 
  last_msec = current_msec; 
 
  //process_midi_cb execution...
  cb(event, nmsecs_since_last, ctx->callback_userdata); 
}

void load_midi_file(char *filename, read_midi_callback callback, void *callback_userdata){ 
  fluid_playlist_item playlist_item;
  playlist_item.filename = filename;

  fluid_player_t *player;
  player = (fluid_player_t *)new_fluid_player();
  player->playback_callback = &get_events; 


  read_midi_ctx_t ctx;
  ctx.player = player;
  ctx.callback = callback;
  ctx.callback_userdata = callback_userdata;

  player->playback_userdata = (void *)&ctx;
  fluid_player_load(player, &playlist_item);

  int i;
  for(i = 0; i < player->ntracks; i++){
     ctx.track = player->track[i];//tracks...when there is more than one song in a file 
     fluid_track_send_events(player->track[i], player->synth, player, (unsigned int)-1); 
  }

  delete_fluid_player(player);
}
