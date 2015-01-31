#define _XOPEN_SOURCE 500

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "jalv_config.h"
#include "LV2-render_internal.h"

#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

static int
print_usage(const char* name, bool error)
{
	FILE* const os = error ? stderr : stdout;
	fprintf(os, "Usage: %s [OPTION...] PLUGIN_URI\n", name);
	fprintf(os, "Render a midi file with an LV2 plugin instrument.\n");
	fprintf(os, "  -h           Display this help and exit\n");
	fprintf(os, "  -i STRING    filename for midi input (DEFAULT=test.mid)\n");
	fprintf(os, "  -o STRING    filename for wavfile output (DEFAULT=output.wav)\n");
	fprintf(os, "  -p           Print control output changes to stdout\n");
	fprintf(os, "  -c SYM=VAL   Set control value (e.g. \"vol=1.4\")\n");
	fprintf(os, "  -C NUM       Integer number of channels for output, e.g. 2 (DEFAULT=2)\n");
	fprintf(os, "  -S NUM       Integer sample rate of output, e.g. 44100 (DEFAULT=48000)\n");
	fprintf(os, "  -l DIR       Load state from save directory\n");
	fprintf(os, "  -d DIR       Dump plugin <=> UI communication\n");
	fprintf(os, "  -b SIZE      Buffer size for plugin <=> UI communication\n");
	return error ? 1 : 0;
}

int
jalv_init(int* argc, char*** argv, JalvOptions* opts)
{
	opts->controls    = malloc(sizeof(char*));
	opts->controls[0] = NULL;

	int n_controls = 0;
	int a          = 1;
	for (; a < *argc && (*argv)[a][0] == '-'; ++a) {
		if ((*argv)[a][1] == 'h') {
			return print_usage((*argv)[0], true);
		} else if ((*argv)[a][1] == 's') {
			opts->show_ui = true;
		} else if ((*argv)[a][1] == 'p') {
			opts->print_controls = true;
		} else if ((*argv)[a][1] == 'l') {
			if (++a == *argc) {
				fprintf(stderr, "Missing argument for -l\n");
				return 1;
			}
			opts->load = jalv_strdup((*argv)[a]);
		} else if ((*argv)[a][1] == 'b') {
			if (++a == *argc) {
				fprintf(stderr, "Missing argument for -b\n");
				return 1;
			}
			opts->buffer_size = atoi((*argv)[a]);
		} else if ((*argv)[a][1] == 'c') {
			if (++a == *argc) {
				fprintf(stderr, "Missing argument for -c\n");
				return 1;
			}
			opts->controls = realloc(opts->controls,
			                         (++n_controls + 1) * sizeof(char*));
			opts->controls[n_controls - 1] = (*argv)[a];
			opts->controls[n_controls]     = NULL;
		} else if ((*argv)[a][1] == 'C') {
			if (++a == *argc) {
				fprintf(stderr, "Missing argument for -C\n");
				return 1;
			}
			opts->nchannels = atoi((*argv)[a]);
		} else if ((*argv)[a][1] == 'S') {
			if (++a == *argc) {
				fprintf(stderr, "Missing argument for -S\n");
				return 1;
			}
			opts->sample_rate = atoi((*argv)[a]);
		} else if ((*argv)[a][1] == 'i') {
			if (++a == *argc) {
				fprintf(stderr, "Missing argument for -i\n");
				return 1;
			}
			opts->infile = (*argv)[a]; 
		} else if ((*argv)[a][1] == 'o') {
			if (++a == *argc) {
				fprintf(stderr, "Missing argument for -o\n");
				return 1;
			}
			opts->outfile = (*argv)[a]; 
		} else if ((*argv)[a][1] == 'd') {
			opts->dump = true;
		} else {
			fprintf(stderr, "Unknown option %s\n", (*argv)[a]);
			return print_usage((*argv)[0], true);
		}
	}

	return 0;
}
