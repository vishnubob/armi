/*  Example of Amplitude Modulation synthesis
 *  using Mozzi sonification library.
 *
 *  Demonstrates modulating the gain of one oscillator
 *  by the instantaneous amplitude of another oscillator,
 *  shows the use of fixed-point numbers to express fractional
 *  values, random numbers with xorshift96(), and EventDelay()
 *  for scheduling.
 *
 *  Circuit: Audio output on digital pin 9.
 *
 *  Mozzi help/discussion/announcements:
 *  https://groups.google.com/forum/#!forum/mozzi-users
 *
 *  Tim Barrass 2012.
 *  This example code is in the public domain.
 */

#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/COS2048_int8.h> // table for Oscils to play
#include <utils.h>
#include <fixedMath.h>
#include <EventDelay.h>
#include <ardumidi.h>

#define CONTROL_RATE 64 // powers of 2 please

// audio oscils
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aCarrier(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aModulator(COS2048_DATA);

// control oscil
Oscil<COS2048_NUM_CELLS, CONTROL_RATE> kCarrierFreqSweeper(COS2048_DATA);

// for scheduling note changes in updateControl()
EventDelay kChangeNoteDelay(CONTROL_RATE);

unsigned char mod_depth = 256;

// synthesis parameters in fixed point formats
Q8n8 ratio; // unsigned int with 8 integer bits and 8 fractional bits
Q24n8 carrier_freq; // unsigned long with 24 integer bits and 8 fractional bits
Q24n8 mod_freq; // unsigned long with 24 integer bits and 8 fractional bits

// for random notes
Q8n0 octave_start_note = 42;

void setup(){
  ratio = float_to_Q8n8(2.0f); // define ratio in float and convert to fixed-point
  kCarrierFreqSweeper.setFreq(0.1f);
  kChangeNoteDelay.set(200); // note duration ms, within resolution of CONTROL_RATE
  startMozzi(CONTROL_RATE);
  Serial.begin(115200);
}

void setFreqs(Q16n16 midi_note){
    carrier_freq = Q16n16_to_Q24n8(Q16n16_mtof(midi_note));
    mod_freq = (carrier_freq * ratio)>>8; // (Q24n8 * Q8n8) >> 8 = Q24n8
    aCarrier.setFreq_Q24n8(carrier_freq);
    aModulator.setFreq_Q24n8(mod_freq);
}

bool onoff = false;

void updateControl(){
    Q16n16 target_note;
  if (!midi_message_available())
    return;
  MidiMessage msg = read_midi_message();

  if (msg.command == MIDI_NOTE_ON)
  {
      if (msg.param2 == 0)
      {
          target_note = Q8n0_to_Q16n16(0);
          onoff = false;
      } else
      {
          target_note = Q8n0_to_Q16n16(msg.param1);
          onoff = true;
      }
  } else 
  if (msg.command == MIDI_NOTE_OFF)
  {
      target_note = Q8n0_to_Q16n16(0);
      onoff = false;
  }

  setFreqs(target_note);
}

int updateAudio(){
  if (!onoff) return 0;
  int out = ((int)aCarrier.next() * ((unsigned char)128+ aModulator.next()))>>8;
  return out;
}

void loop(){
  audioHook();
}
