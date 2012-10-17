#include <arduino.h>
#include "ardumidi.h"

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
}

// the loop routine runs over and over again forever:
void loop() 
{
    static int last_val = 0;
    int sensorValue = analogRead(A0);
    int mappedValue = min(0x7f, max(0, map(sensorValue, 0, 70, 0, 0x7f)));

    if (last_val != sensorValue)
    {
        //Serial.println(mappedValue);
        midi_note_on(0, sensorValue, 0x70);
    }  
    last_val = sensorValue;
}
