#include <arduino.h>
#include "ardumidi.h"

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
}

int get_impulse(int samples=10)
{
    int maxSensorValue = 0;
    for(int x = 0; x < samples; ++x)
    {
        int localSensorValue = analogRead(A0);
        maxSensorValue = max(localSensorValue, maxSensorValue);
    }
    return maxSensorValue;
}
    

// the loop routine runs over and over again forever:
void loop() 
{
    static int last_val = 0;
    //int sensorValue = analogRead(A0);
    int sensorValue = get_impulse(30);
    int mappedValue = min(0x7f, max(0, map(sensorValue, 0, 200, 0, 0x7f)));

    if (last_val != sensorValue)
    {
        //Serial.println(sensorValue);
        //Serial.println(mappedValue);
        midi_note_on(0, 37, sensorValue);
        delay(100);
        midi_note_on(0, 37, 0);
    }  
    last_val = last_val;
}
