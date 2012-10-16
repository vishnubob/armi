#include "ardumidi.h"

void _setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
}

// the loop routine runs over and over again forever:
void __loop() {
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

int ledPin = 13;
int speakerOut = 9;               
byte names[] = {'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C'};  
int tones[] = {1915, 1700, 1519, 1432, 1275, 1136, 1014, 956};
byte melody[] = "2d2a1f2c2d2a2d2c2f2d2a2c2d2a1f2c2d2a2a2g2p8p8p8p";
// count length: 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
//                                10                  20                  30
int count = 0;
int count2 = 0;
int count3 = 0;
int MAX_COUNT = 24;
int statePin = LOW;

void setup() {
 pinMode(ledPin, OUTPUT); 
}

void loop() {
  analogWrite(speakerOut, 0);     
  for (count = 0; count < MAX_COUNT; count++) {
    statePin = !statePin;
    digitalWrite(ledPin, statePin);
    for (count3 = 0; count3 <= (melody[count*2] - 48) * 30; count3++) {
      for (count2=0;count2<8;count2++) {
        if (names[count2] == melody[count*2 + 1]) {       
          analogWrite(speakerOut,500);
          delayMicroseconds(tones[count2]);
          analogWrite(speakerOut, 0);
          delayMicroseconds(tones[count2]);
        } 
        if (melody[count*2 + 1] == 'p') {
          // make a pause of a certain size
          analogWrite(speakerOut, 0);
          delayMicroseconds(500);
        }
      }
    }
  }
}
