typedef struct _fluid_list_t fluid_list_t;

typedef struct _fluid_midi_event_t fluid_midi_event_t;
typedef struct _fluid_player_t fluid_player_t;
typedef struct _fluid_track_t fluid_track_t;
typedef struct _fluid_synth_t fluid_synth_t;
typedef struct _fluid_timer_t fluid_timer_t;
typedef struct _fluid_sample_timer_t fluid_sample_timer_t;
typedef struct _fluid_settings_t fluid_settings_t;


#define FLUID_DBG 0
#define FLUID_OK 1
#define FLUID_ERR 0
#define FLUID_FAILED 2
#define FLUID_PANIC 3
#define FLUID_PLAYER_DONE 2
#define FLUID_PLAYER_PLAYING 1
#define FLUID_PLAYER_READY 3
#define FLUID_HINT_TOGGLED 0x01
#define TRUE 1
#define FALSE 0
typedef int (*handle_midi_event_func_t)(void* data, fluid_midi_event_t* event);
