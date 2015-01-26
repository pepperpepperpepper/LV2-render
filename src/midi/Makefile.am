noinst_LTLIBRARIES = libfluidmidi.la
libfluidmidi_la_SOURCES = fluid_midi.c fluid_list.c midi_loader.c 

bin_PROGRAMS = test_midi_loader

test_midi_loader_SOURCES = test_midi_loader.c
test_midi_loader_LDADD = -lm -ldl libfluidmidi.la
