// alsa_midi_out.c
#include "alsa.h"

int midi_init(MidiOut *m, const char *client_name, const char *port_name) {
   if (snd_seq_open(&m->seq, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
      fprintf(stderr, "snd_seq_open failed\n");
      return -1;
   }

   snd_seq_set_client_name(m->seq, client_name);

   m->out_port = snd_seq_create_simple_port(
      m->seq, port_name, SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
      SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);

   if (m->out_port < 0) {
      fprintf(stderr, "failed to create ALSA MIDI port\n");
      snd_seq_close(m->seq);
      return -1;
   }

   return 0;
}

void midi_send_event(MidiOut *m, snd_seq_event_t *ev) {
   snd_seq_ev_set_source(ev, m->out_port);
   snd_seq_ev_set_subs(ev);
   snd_seq_ev_set_direct(ev);
   snd_seq_event_output_direct(m->seq, ev);
}

void midi_note_on(MidiOut *m, int channel, int note, int velocity) {
   snd_seq_event_t ev;
   snd_seq_ev_clear(&ev);
   snd_seq_ev_set_noteon(&ev, channel, note, velocity);
   midi_send_event(m, &ev);
}

void midi_note_off(MidiOut *m, int channel, int note) {
   snd_seq_event_t ev;
   snd_seq_ev_clear(&ev);
   snd_seq_ev_set_noteoff(&ev, channel, note, 0);
   midi_send_event(m, &ev);
}

void midi_cc(MidiOut *m, int channel, int controller, int value) {
   snd_seq_event_t ev;
   snd_seq_ev_clear(&ev);
   snd_seq_ev_set_controller(&ev, channel, controller, value);
   midi_send_event(m, &ev);
}

void midi_program_change(MidiOut *m, int channel, int program) {
   snd_seq_event_t ev;
   snd_seq_ev_clear(&ev);
   snd_seq_ev_set_pgmchange(&ev, channel, program);
   midi_send_event(m, &ev);
}

void midi_pitch_change(MidiOut *m, int channel, int pitch) {
   snd_seq_event_t ev;
   snd_seq_ev_clear(&ev);
   snd_seq_ev_set_pitchbend(&ev, channel, pitch);
   midi_send_event(m, &ev);
}

void midi_close(MidiOut *m) {
   if (m->seq)
      snd_seq_close(m->seq);
}

void midi_connect(MidiOut *m, int device, int devicePort) {
   snd_seq_connect_to(m->seq, m->out_port, device, devicePort);
}

int main(void) {
   MidiOut midi;
   struct processedPacket input;
   struct processedPacket previous;
   memset(&previous, 0, sizeof previous);

   int prev_active = 0;
   midi_init(&midi, "swhms-midi", "MIDI OUT");
   midi_connect(&midi, 128, 0);
   while (readIO(alsaInPacket, &input, sizeof input) > 0) {
      int curr_active = (input.data1.noteNum > 0 && input.data2.velocity > 0);

      int note_changed = prev_active && curr_active &&
                         input.data1.noteNum != previous.data1.noteNum;

      int prog_changed =
         prev_active && input.data2.midiNum != previous.data2.midiNum;

      int note_started = curr_active && !prev_active;
      int note_stopped = prev_active && !curr_active;

      if (note_stopped || note_changed || prog_changed)
         midi_note_off(&midi, 0, previous.data1.noteNum);

      if (input.data2.midiNum != previous.data2.midiNum)
         midi_program_change(&midi, 0, input.data2.midiNum);

      if (input.data1.pitchBend != previous.data1.pitchBend)
         midi_pitch_change(&midi, 0, input.data1.pitchBend);

      if (input.data2.cc11 != previous.data2.cc11)
         midi_cc(&midi, 0, 11, input.data2.cc11);

      if (note_started || note_changed || prog_changed)
         midi_note_on(&midi, 0, input.data1.noteNum, input.data2.velocity);

      previous = input;
      prev_active = curr_active;
   }
   midi_close(&midi);
   return 0;
}