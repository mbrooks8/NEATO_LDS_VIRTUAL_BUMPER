#include <Adafruit_NeoPixel.h>
const float pi = 3.14159;
short ledNum;

#define PIN 6
// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(39, PIN, NEO_GRB + NEO_KHZ800);
//Set the default colors here
uint32_t lightBlue =  strip.Color(30, 144, 255); // Light Blue
uint32_t royalBlue =  strip.Color(65, 105, 255); // Royal Blue
uint32_t red =  strip.Color(230, 0, 0); // RED
uint32_t magenta = strip.Color(255, 0, 255);
uint32_t greenishwhite = strip.Color(0, 64, 0, 64);
uint32_t green =  strip.Color(0, 255, 0); // green
//Dont touch this
int modifier = 360 / strip.numPixels();
//Change this stuff down here
uint32_t defaultColor = royalBlue;
uint32_t eyeColor = red;
uint32_t errorColor = red;
//This thing moves the eye
void eyeMove(int degree, int eyeSpeed) {
  //Serial.print("movin the eye");
  //We have our initial leds that are lit up we set a new temp led number to move the eye to that direction
  int oldDegree = ledNum * modifier; // get degree from the previus LED number
  //Number of degrees i need to move to get to the new location
  int ritard;
  degree = degree % 360;
  int go;
  ritard = abs(degree - oldDegree);
  if (ritard > 10 ) {
    //we sent tempdegree because we need the value of old degree for the next time we run the function to move the eye
    int tempDegree = oldDegree;

    if (degree - oldDegree > 0 ) {
      //forward
      go = 0;
      if (ritard > 270) {
        ritard = ritard - 270;
        go = 1;
      }
    } else {
      //backwards
      go = 1;
      if (ritard > 270) {
        ritard = ritard - 270;
        go = 0;
      }
    }
    /* Serial.print("ritard:");
      Serial.print(ritard);
      Serial.println();*/
    for (uint16_t i = 0; i < ritard; i++) {
      //we go from tempDegree to the new inputed degree to slowly transition from the current eye to the new eye
      //if the new degree is large than the old degree we will incriment temp degree to move the eye clockwise
      //if the degree is negative then we need to move the eye in the other direction
      if (go == 0) {
        ++tempDegree;
      } else {
        --tempDegree;
      }
      /* Serial.print("tempDegree:");
        Serial.print(tempDegree);
        Serial.println();*/
      eye(tempDegree, eyeColor);
      //Eye speed set in the function call is the total time itll take to complete the entire function. ritard gives us the number of times we need to call eye()
      delay(eyeSpeed / ritard);
    }
  }
}

// Fill Strip with 3 leds to be the eye --> More leds will be used if the strip changes to have more leds
// degree is the degree that you want to move the eye
// c is the color that you want the eye to be
void eye(int degree, uint32_t c) {

  //figure out which led needs to be lit for the main eye led
  ledNum = abs((degree / modifier)) % strip.numPixels();
  //these 3 pixels make up the color of the eye the first one is the main pixel
  strip.setPixelColor(ledNum, c);
  strip.setPixelColor((ledNum + 1) % strip.numPixels(), c);
  strip.setPixelColor((strip.numPixels() + (ledNum - 1))  % strip.numPixels(), c);

  //these two resets the colors when the eye needs to move
  strip.setPixelColor((strip.numPixels() + (ledNum - 2))  % strip.numPixels(), defaultColor);
  strip.setPixelColor((ledNum + 2) % strip.numPixels(), defaultColor);
  strip.show();
}

//this is supposed to pulse the light but its not working --> dont use this
void ringPulse(uint32_t c, uint8_t wait) {
  long starttime = millis();
  long endtime = starttime;
  int i = 0;

  while ((endtime - starttime) <= 10000) // do this loop for up to 1000mS
  {
    if (i = 0) {
      for ( i = 0; i < 80; i++) {
        strip.setBrightness(i);
        colorWipe(c, 0);
        strip.show();
        delay(2);
      }
    } else {
      for ( i = 80; i >= 0; --i) {
        strip.setBrightness(i);
        colorWipe(c, 0);
        strip.show();
        delay(2);
      }
    }
    endtime = millis();
  }

}

// Fill the dots one after the other with a color
void backItUp(uint8_t wait) {
  eyeMove(280, 100);
  uint16_t i, j;
  for (j = 0; j < 256; j++) {
    for (i = 25; i < 38; i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
void setup() {
  // put your setup code here, to run once:
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  colorWipe(defaultColor, 0);
  strip.setBrightness(100);
  strip.show();
}

void loop() {
  // put your main code here, to run repeatedly:
  colorWipe(defaultColor, 0);
  strip.setBrightness(100);
  strip.show();
}
