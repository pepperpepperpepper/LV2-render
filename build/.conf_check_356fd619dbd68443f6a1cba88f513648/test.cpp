#include <jack/jack.h>

int main(int argc, char **argv) {
	void *p;
	(void)argc; (void)argv;
	p=(void*)(jack_port_type_get_buffer_size);
	return 0;
}
