#define _POSIX_C_SOURCE 200809L  /* for mkdtemp */
#define _DARWIN_C_SOURCE /* for mkdtemp on OSX */
#define _DEFAULT_SOURCE
#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __cplusplus
#    include <stdbool.h>
#endif

#ifdef _WIN32
#    include <io.h>  /* for _mktemp */
#    define snprintf _snprintf
#else
#    include <sys/stat.h>
#    include <sys/types.h>
#    include <unistd.h>
#endif

#include "jalv_config.h"
#include "LV2-render_internal.h"


#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/buf-size/buf-size.h"
#include "lv2/lv2plug.in/ns/ext/data-access/data-access.h"
#include "lv2/lv2plug.in/ns/ext/event/event.h"
#include "lv2/lv2plug.in/ns/ext/options/options.h"
#include "lv2/lv2plug.in/ns/ext/parameters/parameters.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/port-groups/port-groups.h"
#include "lv2/lv2plug.in/ns/ext/presets/presets.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include "lv2/lv2plug.in/ns/ext/uri-map/uri-map.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#include "lilv/lilv.h"

#include "lv2_evbuf.h"
#include "worker.h"

#define NS_RDF "http://www.w3.org/1999/02/22-rdf-syntax-ns#"

#define USTR(str) ((const uint8_t*)str)

#ifndef MIN
#    define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#    define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifdef __clang__
#    define REALTIME __attribute__((annotate("realtime")))
#else
#    define REALTIME
#endif


/* Size factor for UI ring buffers.  The ring size is a few times the size of
   an event output to give the UI a chance to keep up.  Experiments with Ingen,
   which can highly saturate its event output, led me to this value.  It
   really ought to be enough for anybody(TM).
*/
#define N_BUFFER_CYCLES 16
#define SAMPLE_RATE 48000 


#include <alsa/asoundlib.h>
#include <sndfile.h>
#include "midi/midi_loader.h"
#include "midi/fluidsynth_priv.h"

int min(int x, int y) {
  return (x < y) ? x : y;
}

typedef struct process_midi_ctx_t {
   Jalv *jalv;
   SNDFILE *outfile;
   float  sample_rate;
} process_midi_ctx_t;


void print_audio_to_terminal(float *sf_output, size_t nframes){
    size_t i;
    for(i = 0; i< nframes; i++){ 
      printf("%04x ", * ((unsigned int *)sf_output) );
      sf_output++;
    }
}

int write_audio_to_file(SNDFILE *outfile, float *sf_output, size_t nframes){ 
    size_t items_written = 0;
    /* Write the audio */
    if ((items_written = sf_writef_float(outfile, 
					 sf_output,
					 nframes)) != nframes) {
      fprintf(stderr, "Error: can't write data to output file\n");
      fprintf(stderr, "%s: %s\n", "jalv", sf_strerror(outfile));
      return 1;
    }
  return 0; 
}

SNDFILE *open_wav_file(char *output_file, float sample_rate, int nchannels, size_t length){
  /* prepare file */
  SF_INFO outsfinfo;
  SNDFILE *outfile; 

  outsfinfo.samplerate = sample_rate;
  outsfinfo.channels = nchannels;
  outsfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

  outsfinfo.frames = length;

  outfile = sf_open(output_file, SFM_WRITE, &outsfinfo);
  return outfile;
}


int process_midi_cb(fluid_midi_event_t *event, size_t msecs, process_midi_ctx_t *ctx) 
{
  Jalv *jalv = ctx->jalv; 
  float **pluginAudioIOBuffers = (float **)calloc(jalv->num_ports, sizeof(float *));
  float *pluginAudioPtrs[100];
  size_t pluginAudioOutputCount = 0;
  size_t nframes;

  /* convert msecs */
  nframes = msecs * ctx->sample_rate / 1000; 

	/* Prepare port buffers */
	for (uint32_t p = 0; p < jalv->num_ports; ++p) {
		struct Port* port = &jalv->ports[p];
		if (port->type == TYPE_AUDIO) { 
      pluginAudioIOBuffers[p] = (float *)calloc(nframes, sizeof(float));
			lilv_instance_connect_port( 
				jalv->instance, p, //connect port p to this location
        pluginAudioIOBuffers[p]
      );
      if (port->flow == FLOW_OUTPUT){
        pluginAudioPtrs[pluginAudioOutputCount] = pluginAudioIOBuffers[p];
        pluginAudioOutputCount++;
        printf("buffer %x ptr: %8x\n", p, pluginAudioIOBuffers[p]);
      } 
		} else if (port->type == TYPE_EVENT && port->flow == FLOW_INPUT) {
			lv2_evbuf_reset(port->evbuf, true);

			LV2_Evbuf_Iterator iter = lv2_evbuf_begin(port->evbuf);
       
      uint8_t midi_event_buffer[3];
          midi_event_buffer[0] = event->type;
          midi_event_buffer[1] = event->param1;
          midi_event_buffer[2] = event->param2;
//atom_event: type: 19 frames: 1013 size: 3 
					lv2_evbuf_write(&iter,
					                0, 0, //Doesn't care about these, not sure why
					                jalv->midi_event_id, 
					                sizeof(midi_event_buffer), midi_event_buffer);
		} else if (port->type == TYPE_EVENT) {
			/* Clear event output for plugin to write to */
		}
	}


	lilv_instance_run(jalv->instance, nframes); 
//TODO
//    /* Interleaving for libsndfile. */ 
    int nchannels = 2; 
    if (nchannels > pluginAudioOutputCount){
      fprintf(stderr, "ERROR: Requesting more audio outputs than available from plugin.\n");
      exit(1);
    }
    float sf_output[nchannels * nframes]; //nframes is n times longer now
    for (int i = 0; i < nframes; i++) {
      /* First, write all the obvious channels */
      /* If outs > nchannels, we *could* do mixing - but don't. */
      //actually you need another for loop in here for 10 channel wavs
//      sf_output[i * nchannels + 0] = pluginAudioIOBuffers[3][i];
//      sf_output[i * nchannels + 1] = pluginAudioIOBuffers[4][i];
      for (size_t n = 0; n < pluginAudioOutputCount; n++){
        sf_output[i * nchannels + n] = pluginAudioPtrs[n][i];
      }
      /* Then, if user wants *more* output channels than there are
       * audio output ports (ie outs < nchannels), copy the last audio
       * out to all the remaining channels. If outs >= nchannels, this
       * loop is never entered. */
    }

    write_audio_to_file(ctx->outfile, sf_output, nframes); 
    for(int i=0; i<jalv->num_ports; i++){
      if(pluginAudioIOBuffers[i]){
        free(pluginAudioIOBuffers[i]);
      }
    }

	return 0;
}


ZixSem exit_sem;  /**< Exit semaphore */

static LV2_URID
map_uri(LV2_URID_Map_Handle handle,
        const char*         uri)
{
	Jalv* jalv = (Jalv*)handle;
	zix_sem_wait(&jalv->symap_lock);
	const LV2_URID id = symap_map(jalv->symap, uri);
	zix_sem_post(&jalv->symap_lock);
	return id;
}

static const char*
unmap_uri(LV2_URID_Unmap_Handle handle,
          LV2_URID              urid)
{
	Jalv* jalv = (Jalv*)handle;
	zix_sem_wait(&jalv->symap_lock);
	const char* uri = symap_unmap(jalv->symap, urid);
	zix_sem_post(&jalv->symap_lock);
	return uri;
}

/**
   Map function for URI map extension.
*/
static uint32_t
uri_to_id(LV2_URI_Map_Callback_Data callback_data,
          const char*               map,
          const char*               uri)
{
	Jalv* jalv = (Jalv*)callback_data;
	zix_sem_wait(&jalv->symap_lock);
	const LV2_URID id = symap_map(jalv->symap, uri);
	zix_sem_post(&jalv->symap_lock);
	return id;
}



//{{{ LV2 host features defined
#define NS_EXT "http://lv2plug.in/ns/ext/"
static LV2_URI_Map_Feature uri_map = { NULL, &uri_to_id };

static LV2_Extension_Data_Feature ext_data = { NULL };


static LV2_Feature uri_map_feature   = { NS_EXT "uri-map", &uri_map };
static LV2_Feature map_feature       = { LV2_URID__map, NULL };
static LV2_Feature unmap_feature     = { LV2_URID__unmap, NULL };
static LV2_Feature make_path_feature = { LV2_STATE__makePath, NULL };
static LV2_Feature schedule_feature  = { LV2_WORKER__schedule, NULL };
static LV2_Feature log_feature       = { LV2_LOG__log, NULL };
static LV2_Feature options_feature   = { LV2_OPTIONS__options, NULL };
static LV2_Feature def_state_feature = { LV2_STATE__loadDefaultState, NULL };


/** These features have no data */
static LV2_Feature buf_size_features[3] = {
	{ LV2_BUF_SIZE__powerOf2BlockLength, NULL },
	{ LV2_BUF_SIZE__fixedBlockLength, NULL },
	{ LV2_BUF_SIZE__boundedBlockLength, NULL } };

const LV2_Feature* features[13] = {
	&uri_map_feature, &map_feature, &unmap_feature,
	&make_path_feature,
	&schedule_feature,
	&log_feature,
	&options_feature,
	&def_state_feature,
	&buf_size_features[0],
	&buf_size_features[1],
	&buf_size_features[2],
	NULL
};

/** Return true iff Jalv supports the given feature. */
static bool
feature_is_supported(const char* uri)
{
	if (!strcmp(uri, "http://lv2plug.in/ns/lv2core#isLive")) {
		return true;
	}
	for (const LV2_Feature*const* f = features; *f; ++f) {
		if (!strcmp(uri, (*f)->URI)) {
			return true;
		}
	}
	return false;
}

//}}}

/** Abort and exit on error */
static void
die(const char* msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

static void
create_port(Jalv*    jalv,
            uint32_t port_index,
            float    default_value)
{
	struct Port* const port = &jalv->ports[port_index];

	port->lilv_port = lilv_plugin_get_port_by_index(jalv->plugin, port_index);
	port->evbuf     = NULL;
	port->buf_size  = 0;
	port->index     = port_index;
	port->control   = 0.0f;
	port->flow      = FLOW_UNKNOWN;

	const bool optional = lilv_port_has_property(
		jalv->plugin, port->lilv_port, jalv->nodes.lv2_connectionOptional);

	/* Set the port flow (input or output) */
	if (lilv_port_is_a(jalv->plugin, port->lilv_port, jalv->nodes.lv2_InputPort)) {
		port->flow = FLOW_INPUT;
	} else if (lilv_port_is_a(jalv->plugin, port->lilv_port,
	                          jalv->nodes.lv2_OutputPort)) {
		port->flow = FLOW_OUTPUT;
	} else if (!optional) {
		die("Mandatory port has unknown type (neither input nor output)");
	}

	/* Set control values */
	if (lilv_port_is_a(jalv->plugin, port->lilv_port, jalv->nodes.lv2_ControlPort)) {
		port->type    = TYPE_CONTROL;
		port->control = isnan(default_value) ? 0.0f : default_value;
	} else if (lilv_port_is_a(jalv->plugin, port->lilv_port,
	                          jalv->nodes.lv2_AudioPort)) {
		port->type = TYPE_AUDIO;
	} else if (lilv_port_is_a(jalv->plugin, port->lilv_port,
	                          jalv->nodes.ev_EventPort)) {
		port->type = TYPE_EVENT;
		port->old_api = true;
	} else if (lilv_port_is_a(jalv->plugin, port->lilv_port,
	                          jalv->nodes.atom_AtomPort)) {
		port->type = TYPE_EVENT;
		port->old_api = false;
	} else if (!optional) {
		die("Mandatory port has unknown data type");
	}

	LilvNode* min_size = lilv_port_get(
		jalv->plugin, port->lilv_port, jalv->nodes.rsz_minimumSize);
	if (min_size && lilv_node_is_int(min_size)) {
		port->buf_size = lilv_node_as_int(min_size);
		jalv->opts.buffer_size = MAX(
			jalv->opts.buffer_size, port->buf_size * N_BUFFER_CYCLES);
	}
	lilv_node_free(min_size);

	/* Update longest symbol for aligned console printing */
	const LilvNode* sym = lilv_port_get_symbol(jalv->plugin, port->lilv_port);
	const size_t    len = strlen(lilv_node_as_string(sym));
	if (len > jalv->longest_sym) {
		jalv->longest_sym = len;
	}
}

/**
   Create port structures from data (via create_port()) for all ports.
*/
void
jalv_create_ports(Jalv* jalv)
{
	jalv->num_ports = lilv_plugin_get_num_ports(jalv->plugin);
	jalv->ports     = (struct Port*)calloc(jalv->num_ports, sizeof(struct Port));
	float* default_values = (float*)calloc(
		lilv_plugin_get_num_ports(jalv->plugin), sizeof(float));
	lilv_plugin_get_port_ranges_float(jalv->plugin, NULL, NULL, default_values);

	for (uint32_t i = 0; i < jalv->num_ports; ++i) {
		create_port(jalv, i, default_values[i]);
	}

	const LilvPort* control_input = lilv_plugin_get_port_by_designation(
		jalv->plugin, jalv->nodes.lv2_InputPort, jalv->nodes.lv2_control);
	if (control_input) {
		jalv->control_in = lilv_port_get_index(jalv->plugin, control_input);
	}

	free(default_values);
}

/**
   Allocate port buffers (only necessary for MIDI).
*/
static void
jalv_allocate_port_buffers(Jalv* jalv)
{
	for (uint32_t i = 0; i < jalv->num_ports; ++i) {
		struct Port* const port = &jalv->ports[i];
		switch (port->type) {
		case TYPE_EVENT:
			lv2_evbuf_free(port->evbuf);
			const size_t buf_size = (port->buf_size > 0)
				? port->buf_size
				: jalv->midi_buf_size;
			port->evbuf = lv2_evbuf_new(
				buf_size,
				port->old_api ? LV2_EVBUF_EVENT : LV2_EVBUF_ATOM,
				jalv->map.map(jalv->map.handle,
				              lilv_node_as_string(jalv->nodes.atom_Chunk)),
				jalv->map.map(jalv->map.handle,
				              lilv_node_as_string(jalv->nodes.atom_Sequence)));
      //FIXME instance setup here
			lilv_instance_connect_port(
				jalv->instance, i, lv2_evbuf_get_buffer(port->evbuf));
		default: break;
		}
	}
}

/**
   Get a port structure by symbol.

   TODO: Build an index to make this faster, currently O(n) which may be
   a problem when restoring the state of plugins with many ports.
*/
struct Port*
jalv_port_by_symbol(Jalv* jalv, const char* sym)
{
	for (uint32_t i = 0; i < jalv->num_ports; ++i) {
		struct Port* const port     = &jalv->ports[i];
		const LilvNode*    port_sym = lilv_port_get_symbol(jalv->plugin,
		                                                   port->lilv_port);

		if (!strcmp(lilv_node_as_string(port_sym), sym)) {
			return port;
		}
	}

	return NULL;
}

static void
print_control_value(Jalv* jalv, const struct Port* port, float value)
{
	const LilvNode* sym = lilv_port_get_symbol(jalv->plugin, port->lilv_port);
	printf("%-*s = %f\n", jalv->longest_sym, lilv_node_as_string(sym), value);
}
//}}}

static void
activate_port(Jalv*    jalv,
              uint32_t port_index)
{
	struct Port* const port = &jalv->ports[port_index];

	const LilvNode* sym = lilv_port_get_symbol(jalv->plugin, port->lilv_port);

	/* Connect unsupported ports to NULL (known to be optional by this point) */
	if (port->flow == FLOW_UNKNOWN || port->type == TYPE_UNKNOWN) {
		lilv_instance_connect_port(jalv->instance, port_index, NULL);
		return;
	}

	/* Connect the port based on its type */
	switch (port->type) {
	case TYPE_CONTROL:
		print_control_value(jalv, port, port->control);
		lilv_instance_connect_port(jalv->instance, port_index, &port->control);
		break;
	case TYPE_AUDIO:
    //FIXME maybe connect the ports to the buffers here instead
		break;
	case TYPE_EVENT:
		if (lilv_port_supports_event(
			    jalv->plugin, port->lilv_port, jalv->nodes.midi_MidiEvent)) {
		}
		break;
	default:
		break;
	}

}

static bool
jalv_apply_control_arg(Jalv* jalv, const char* s)
{
	char  sym[256];
	float val = 0.0f;
	if (sscanf(s, "%[^=]=%f", sym, &val) != 2) {
		fprintf(stderr, "warning: Ignoring invalid value `%s'\n", s);
		return false;
	}
	
	struct Port* port = jalv_port_by_symbol(jalv, sym);
	if (!port) {
		fprintf(stderr, "warning: Ignoring value for unknown port `%s'\n", sym);
		return false;
	}

	port->control = val;
	return true;
}

static void
signal_handler(int ignored)
{
	zix_sem_post(&exit_sem);
}

int
main(int argc, char** argv)
{
	Jalv jalv;
	memset(&jalv, '\0', sizeof(Jalv));
	jalv.prog_name     = argv[0];
	jalv.block_length  = 4096;  
	jalv.midi_buf_size = 1024; 
	jalv.play_state    = JALV_PAUSED; 
	jalv.bpm           = 120.0f;

	if (jalv_init(&argc, &argv, &jalv.opts)) {
		return EXIT_FAILURE;
	} 

	if (jalv.opts.uuid) {
		printf("UUID: %s\n", jalv.opts.uuid);
	}

	jalv.symap = symap_new(); 
	zix_sem_init(&jalv.symap_lock, 1); 
	uri_map.callback_data = &jalv;

	jalv.map.handle  = &jalv;
	jalv.map.map     = map_uri;
	map_feature.data = &jalv.map;

	jalv.unmap.handle  = &jalv;
	jalv.unmap.unmap   = unmap_uri;
	unmap_feature.data = &jalv.unmap;

	lv2_atom_forge_init(&jalv.forge, &jalv.map);

	jalv.sratom    = sratom_new(&jalv.map);
	jalv.ui_sratom = sratom_new(&jalv.map);

	jalv.midi_event_id = uri_to_id(
		&jalv, "http://lv2plug.in/ns/ext/event", LV2_MIDI__MidiEvent);

	jalv.urids.atom_Float           = symap_map(jalv.symap, LV2_ATOM__Float);
	jalv.urids.atom_Int             = symap_map(jalv.symap, LV2_ATOM__Int);
	jalv.urids.atom_eventTransfer   = symap_map(jalv.symap, LV2_ATOM__eventTransfer);
	jalv.urids.bufsz_maxBlockLength = symap_map(jalv.symap, LV2_BUF_SIZE__maxBlockLength);
	jalv.urids.bufsz_minBlockLength = symap_map(jalv.symap, LV2_BUF_SIZE__minBlockLength);
	jalv.urids.bufsz_sequenceSize   = symap_map(jalv.symap, LV2_BUF_SIZE__sequenceSize);
	jalv.urids.log_Trace            = symap_map(jalv.symap, LV2_LOG__Trace);
	jalv.urids.midi_MidiEvent       = symap_map(jalv.symap, LV2_MIDI__MidiEvent);
	jalv.urids.param_sampleRate     = symap_map(jalv.symap, LV2_PARAMETERS__sampleRate);
	jalv.urids.patch_Set            = symap_map(jalv.symap, LV2_PATCH__Set);
	jalv.urids.patch_property       = symap_map(jalv.symap, LV2_PATCH__property);
	jalv.urids.patch_value          = symap_map(jalv.symap, LV2_PATCH__value);
	jalv.urids.time_Position        = symap_map(jalv.symap, LV2_TIME__Position);
	jalv.urids.time_bar             = symap_map(jalv.symap, LV2_TIME__bar);
	jalv.urids.time_barBeat         = symap_map(jalv.symap, LV2_TIME__barBeat);
	jalv.urids.time_beatUnit        = symap_map(jalv.symap, LV2_TIME__beatUnit);
	jalv.urids.time_beatsPerBar     = symap_map(jalv.symap, LV2_TIME__beatsPerBar);
	jalv.urids.time_beatsPerMinute  = symap_map(jalv.symap, LV2_TIME__beatsPerMinute);
	jalv.urids.time_frame           = symap_map(jalv.symap, LV2_TIME__frame);
	jalv.urids.time_speed           = symap_map(jalv.symap, LV2_TIME__speed);
	jalv.urids.ui_updateRate        = symap_map(jalv.symap, LV2_UI__updateRate);



#ifdef _WIN32
	jalv.temp_dir = jalv_strdup("jalvXXXXXX");
	_mktemp(jalv.temp_dir);
#else
	char* templ = jalv_strdup("/tmp/jalv-XXXXXX");
	jalv.temp_dir = jalv_strjoin(mkdtemp(templ), "/");
	free(templ);
#endif

	LV2_State_Make_Path make_path = { &jalv, jalv_make_path }; 
	make_path_feature.data = &make_path;

	LV2_Worker_Schedule schedule = { &jalv, jalv_worker_schedule }; 
	schedule_feature.data = &schedule;

	LV2_Log_Log llog = { &jalv, jalv_printf, jalv_vprintf };
	log_feature.data = &llog;

	zix_sem_init(&exit_sem, 0);
	jalv.done = &exit_sem;

	zix_sem_init(&jalv.paused, 0);
//	zix_sem_init(&jalv.worker.sem, 0);

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	/* Find all installed plugins */
	LilvWorld* world = lilv_world_new();
	lilv_world_load_all(world);
	jalv.world = world;
	const LilvPlugins* plugins = lilv_world_get_all_plugins(world);

	/* Cache URIs for concepts we'll use */
	jalv.nodes.atom_AtomPort          = lilv_new_uri(world, LV2_ATOM__AtomPort);
	jalv.nodes.atom_Chunk             = lilv_new_uri(world, LV2_ATOM__Chunk);
	jalv.nodes.atom_Sequence          = lilv_new_uri(world, LV2_ATOM__Sequence);
	jalv.nodes.ev_EventPort           = lilv_new_uri(world, LV2_EVENT__EventPort);
	jalv.nodes.lv2_AudioPort          = lilv_new_uri(world, LV2_CORE__AudioPort);
	jalv.nodes.lv2_ControlPort        = lilv_new_uri(world, LV2_CORE__ControlPort);
	jalv.nodes.lv2_InputPort          = lilv_new_uri(world, LV2_CORE__InputPort);
	jalv.nodes.lv2_OutputPort         = lilv_new_uri(world, LV2_CORE__OutputPort);
	jalv.nodes.lv2_connectionOptional = lilv_new_uri(world, LV2_CORE__connectionOptional);
	jalv.nodes.lv2_control            = lilv_new_uri(world, LV2_CORE__control);
	jalv.nodes.lv2_name               = lilv_new_uri(world, LV2_CORE__name);
	jalv.nodes.midi_MidiEvent         = lilv_new_uri(world, LV2_MIDI__MidiEvent);
	jalv.nodes.pg_group               = lilv_new_uri(world, LV2_PORT_GROUPS__group);
	jalv.nodes.pset_Preset            = lilv_new_uri(world, LV2_PRESETS__Preset);
	jalv.nodes.rdfs_label             = lilv_new_uri(world, LILV_NS_RDFS "label");
	jalv.nodes.rsz_minimumSize        = lilv_new_uri(world, LV2_RESIZE_PORT__minimumSize);
	jalv.nodes.work_interface         = lilv_new_uri(world, LV2_WORKER__interface);
	jalv.nodes.work_schedule          = lilv_new_uri(world, LV2_WORKER__schedule);
	jalv.nodes.end                    = NULL;

	/* Get plugin URI from loaded state or command line */
	LilvState* state      = NULL;
	LilvNode*  plugin_uri = NULL;
	if (jalv.opts.load) {
		struct stat info;
		stat(jalv.opts.load, &info);
		if (S_ISDIR(info.st_mode)) {
			char* path = jalv_strjoin(jalv.opts.load, "/state.ttl");
			state = lilv_state_new_from_file(jalv.world, &jalv.map, NULL, path);
			free(path);
		} else {
			state = lilv_state_new_from_file(jalv.world, &jalv.map, NULL,
			                                 jalv.opts.load);
		}
		if (!state) {
			fprintf(stderr, "Failed to load state from %s\n", jalv.opts.load);
			return EXIT_FAILURE;
		}
		plugin_uri = lilv_node_duplicate(lilv_state_get_plugin_uri(state));
	} else if (argc > 1) {
		plugin_uri = lilv_new_uri(world, argv[argc - 1]);
	}

	if (!plugin_uri) {
		fprintf(stderr, "Missing plugin URI, try lv2ls to list plugins\n");
		return EXIT_FAILURE;
	}

	/* Find plugin */
	printf("Plugin:       %s\n", lilv_node_as_string(plugin_uri));
	jalv.plugin = lilv_plugins_get_by_uri(plugins, plugin_uri);
	lilv_node_free(plugin_uri);
	if (!jalv.plugin) {
		fprintf(stderr, "Failed to find plugin\n");
		lilv_world_free(world);
		return EXIT_FAILURE;
	}

	/* Check that any required features are supported */
	LilvNodes* req_feats = lilv_plugin_get_required_features(jalv.plugin);
	LILV_FOREACH(nodes, f, req_feats) {
		const char* uri = lilv_node_as_uri(lilv_nodes_get(req_feats, f));
		if (!feature_is_supported(uri)) {
			fprintf(stderr, "Feature %s is not supported\n", uri);
			lilv_world_free(world);
			return EXIT_FAILURE;
		}
	}
	lilv_nodes_free(req_feats);

	if (!state) {
    printf("Creating new default state for plugin\n");
		/* Not restoring state, load the plugin as a preset to get default */
		state = lilv_state_new_from_world(
			jalv.world, &jalv.map, lilv_plugin_get_uri(jalv.plugin));
	}

	/* Create port structures (jalv.ports) */
	jalv_create_ports(&jalv);

//a lilvnode is just basically any plugin configuration parameter...
	/* Get the plugin's name */
	LilvNode*   name     = lilv_plugin_get_name(jalv.plugin);
	const char* name_str = lilv_node_as_string(name);

	lilv_node_free(name);

	jalv.sample_rate = SAMPLE_RATE; 
	jalv.block_length = 256; //TODO used to be 256 try 1024 4096
  jalv.midi_buf_size = 32768; //used to be 256

	printf("Block length: %u frames\n", jalv.block_length);
	printf("MIDI buffers: %zu bytes\n", jalv.midi_buf_size);

	if (jalv.opts.buffer_size == 0) {
		/* The UI ring is fed by plugin output ports (usually one), and the UI
		   updates roughly once per cycle.  The ring size is a few times the
		   size of the MIDI output to give the UI a chance to keep up.  The UI
		   should be able to keep up with 4 cycles, and tests show this works
		   for me, but this value might need increasing to avoid overflows.
		*/
		jalv.opts.buffer_size = jalv.midi_buf_size * N_BUFFER_CYCLES;
	}

	/* The UI can only go so fast, clamp to reasonable limits */
	jalv.ui_update_hz     = MIN(60, jalv.ui_update_hz);
	jalv.opts.buffer_size = MAX(4096, jalv.opts.buffer_size);
	fprintf(stderr, "Comm buffers: %d bytes\n", jalv.opts.buffer_size);
	fprintf(stderr, "Update rate:  %.01f Hz\n", jalv.ui_update_hz);

	/* Build options array to pass to plugin */
	const LV2_Options_Option options[] = {
		{ LV2_OPTIONS_INSTANCE, 0, jalv.urids.param_sampleRate,
		  sizeof(float), jalv.urids.atom_Float, &jalv.sample_rate },
		{ LV2_OPTIONS_INSTANCE, 0, jalv.urids.bufsz_minBlockLength,
		  sizeof(int32_t), jalv.urids.atom_Int, &jalv.block_length },
		{ LV2_OPTIONS_INSTANCE, 0, jalv.urids.bufsz_maxBlockLength,
		  sizeof(int32_t), jalv.urids.atom_Int, &jalv.block_length },
		{ LV2_OPTIONS_INSTANCE, 0, jalv.urids.bufsz_sequenceSize,
		  sizeof(int32_t), jalv.urids.atom_Int, &jalv.midi_buf_size },
		{ LV2_OPTIONS_INSTANCE, 0, jalv.urids.ui_updateRate,
		  sizeof(float), jalv.urids.atom_Float, &jalv.ui_update_hz },
		{ LV2_OPTIONS_INSTANCE, 0, 0, 0, 0, NULL }
	};
	
	options_feature.data = &options;

	/* Instantiate the plugin */
	jalv.instance = lilv_plugin_instantiate(
		jalv.plugin, jalv.sample_rate, features);
	if (!jalv.instance) {
		die("Failed to instantiate plugin.\n");
	}

	ext_data.data_access = lilv_instance_get_descriptor(jalv.instance)->extension_data;

	fprintf(stderr, "\n");
	if (!jalv.buf_size_set) {
		jalv_allocate_port_buffers(&jalv);
	}

	/* Apply loaded state to plugin instance if necessary */
	if (state) {
		jalv_apply_state(&jalv, state);
    printf("applying state here \n");
	}

	if (jalv.opts.controls) {
		for (char** c = jalv.opts.controls; *c; ++c) {
			jalv_apply_control_arg(&jalv, *c);
		}
	}

	for (uint32_t i = 0; i < jalv.num_ports; ++i) {
		activate_port(&jalv, i);
	}

	/* Activate plugin */
	lilv_instance_activate(jalv.instance);


//FIXME get sample rate from above...right? yes
	jalv.sample_rate = SAMPLE_RATE; 
	jalv.play_state  = JALV_RUNNING;

  // open_wav_file here
  char *output_file = "output.wav";
  size_t length = SAMPLE_RATE; //gets changed when file is closed
  float sample_rate = SAMPLE_RATE; 
  int nchannels = 2;
  SNDFILE *outfile = open_wav_file(output_file, sample_rate, nchannels, length);
  process_midi_ctx_t process_midi_ctx;
  process_midi_ctx.jalv = &jalv;
  process_midi_ctx.outfile = outfile;
  process_midi_ctx.sample_rate = sample_rate;

  load_midi_file("short_example.mid", (read_midi_callback)process_midi_cb, &process_midi_ctx);
//
//STUDY LATER


  sf_close(outfile);

	fprintf(stderr, "Exiting...\n");

	/* Terminate the worker */

	for (uint32_t i = 0; i < jalv.num_ports; ++i) {
		if (jalv.ports[i].evbuf) {
			lv2_evbuf_free(jalv.ports[i].evbuf);
		}
	}

	/* Deactivate plugin */
	lilv_instance_deactivate(jalv.instance);
	lilv_instance_free(jalv.instance);

	/* Clean up */
	free(jalv.ports);
	for (LilvNode** n = (LilvNode**)&jalv.nodes; *n; ++n) {
		lilv_node_free(*n);
	}
	symap_free(jalv.symap);
	zix_sem_destroy(&jalv.symap_lock);
	sratom_free(jalv.sratom);
	sratom_free(jalv.ui_sratom);
	lilv_world_free(world);

	zix_sem_destroy(&exit_sem);

	remove(jalv.temp_dir);
	free(jalv.temp_dir);
	free(jalv.ui_event_buf);

	return 0;
}

