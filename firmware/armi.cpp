#include "ardumidi.h"

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
}

// the loop routine runs over and over again forever:
void loop() {
  static int last_val = 0;
  int sensorValue = min(0x7f, max(0, map(analogRead(A0), 0, 450, 0, 0x7f)));

  if (last_val != sensorValue)
  {
    //Serial.println(sensorValue);
    midi_note_on(0, sensorValue, 0x70);
  }  
  
  last_val = sensorValue;
}

void _loop() {
  static int last_val = 0;
  static bool playing = false;
  int sensorValue = analogRead(A0);
  if (abs(last_val - sensorValue) > 1)
  {
    Serial.println(sensorValue);
    last_val = sensorValue;
    delay(10);
    return;
    if(!playing)
    {
      midi_note_on(0, 0x40, 0x20);
      playing = true;
    }
  }
  if (playing)
  {
      midi_key_pressure(0, 0x40, map(sensorValue, 0, 600, 0, 0x7f));
  }
  if(sensorValue < 10 && playing)
  {
    playing = false;
    midi_note_off(0, 0x40, 0x00);
  }  
  
  last_val = sensorValue;
}
