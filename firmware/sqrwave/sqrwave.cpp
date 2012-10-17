#include <arduino.h>

void setup()
{
}

void loop()
{
    tone(9, 440, 100);
    delay(1000);
    for(int x = 0; x < 1000; ++x)
    {
        tone(9, x, 100);
    }
    delay(1000);
    for(int x = 1000; x >= 0; --x)
    {
        tone(9, x, 100);
    }
    delay(1000);
}
