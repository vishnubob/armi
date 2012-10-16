#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/cos2048_int8.h> // table for Oscils to play
#include <utils.h>
#include <fixedMath.h>
#include <EventDelay.h>
#include <Smooth.h>
#include "../ardumidi.h"

#define CONTROL_RATE 256 // powers of 2 please

Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aCarrier(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aModulator(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, CONTROL_RATE> kModIndex(COS2048_DATA);

// The ratio of deviation to modulation frequency is called the "index of modulation". ( I = d / Fm )
// It will vary according to the frequency that is modulating the carrier and the amount of deviation.
// so deviation d = I * Fm
// haven't quite worked this out properly yet...

Q8n8 mod_index;// = float_to_Q8n8(2.0f); // constant version
Q16n16 deviation;

Q16n16 carrier_freq, mod_freq;

// FM ratio between oscillator frequencies, stays the same through note range
Q8n8 mod_to_carrier_ratio = float_to_Q8n8(3.f);

EventDelay kNoteChangeDelay(CONTROL_RATE);

// for note changes
Q7n8 target_note, note0, note1, note_upper_limit, note_lower_limit, note_change_step, smoothed_note;

// using Smooth on midi notes rather than frequency, 
// because fractional frequencies need larger types than Smooth can handle
// Inefficient, but...until there is a better Smooth....
Smooth <int> kSmoothNote(0.95f);

void setup(){
  Serial.begin(115200);
  kNoteChangeDelay.set(768); // ms countdown, taylored to resolution of CONTROL_RATE
  kModIndex.setFreq(.768f); // sync with kNoteChangeDelay
  target_note = note0;
  note_change_step = Q7n0_to_Q7n8(3);
  note_upper_limit = Q7n0_to_Q7n8(50);
  note_lower_limit = Q7n0_to_Q7n8(32);
  note0 = note_lower_limit;
  note1 = note_lower_limit + Q7n0_to_Q7n8(5);
  startMozzi(CONTROL_RATE);
}

void setFreqs(Q8n8 midi_note){
  carrier_freq = Q16n16_mtof(Q8n8_to_Q16n16(midi_note)); // convert midi note to fractional frequency
  mod_freq = ((carrier_freq>>8) * mod_to_carrier_ratio)  ; // (Q16n16>>8) * Q8n8 = Q16n16, beware of overflow
  deviation = ((mod_freq>>16) * mod_index); // (Q16n16>>16) * Q8n8 = Q24n8, beware of overflow
  aCarrier.setFreq_Q16n16(carrier_freq);
  aModulator.setFreq_Q16n16(mod_freq);
}

void updateControl(){
  if (!midi_message_available())
    return;
  MidiMessage msg = read_midi_message();

  if (msg.command == MIDI_NOTE_ON)
  {
      if (msg.param2 == 0)
      {
          target_note = Q7n0_to_Q7n8(0);
      } else
      {
          target_note = Q7n0_to_Q7n8(msg.param1);
      }
  } else 
  if (msg.command == MIDI_NOTE_OFF)
  {
      target_note = Q7n0_to_Q7n8(0);
  }

  // vary the modulation index
  //mod_index = (Q8n8)350+kModIndex.next();
  
  // here's where the smoothing happens
  smoothed_note = kSmoothNote.next(target_note);
  //setFreqs(smoothed_note);
  setFreqs(target_note);
}

int updateAudio(){
  Q15n16 modulation = deviation * aModulator.next() >> 8;
  return (int)aCarrier.phMod(modulation);
}

void loop(){
  audioHook();
}
