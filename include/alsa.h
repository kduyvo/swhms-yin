#ifndef ALSA_H
#define ALSA_H
#include "helperIO.h"
#include "midiStructs.h"
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
   snd_seq_t *seq;
   int out_port;
} MidiOut;

struct alsa_device {
   int client;
   char name[128];
   int ports[8];
   char ports_names[128][8];
   int ports_size;
   int is_kernel;
};

enum fdAlsa {
   alsaInPacket = 100,
   alsaInPort,
};

int midi_init(MidiOut *m, const char *client_name, const char *port_name);
void midi_send_event(MidiOut *m, snd_seq_event_t *ev);
void midi_note_on(MidiOut *m, int channel, int note, int velocity);
void midi_note_off(MidiOut *m, int channel, int note);
void midi_cc(MidiOut *m, int channel, int controller, int value);
void midi_program_change(MidiOut *m, int channel, int program);
void midi_close(MidiOut *m);
#endif