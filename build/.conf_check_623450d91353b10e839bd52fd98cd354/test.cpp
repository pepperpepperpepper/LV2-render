#include <jack/metadata.h>

int main(int argc, char **argv) {
	void *p;
	(void)argc; (void)argv;
	p=(void*)(jack_set_property);
	return 0;
}
