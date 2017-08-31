#define distance 10
#define distanceFromCenterOfRotationTolazer 2.5
#include <Adafruit_NeoPixel.h>
const float pi = 3.14159;

#define PIN 6
short ledNum;

// LDS receive buffer
#define RXBUFF_SZ 22
#define START_CHAR 0xFA
#define INDEX_BYTE 1
#define INDEX_BIAS 0xA0
#define SPEED_L 2
#define SPEED_H 3
#define DATA_START 4
#define CKL 20
#define CKH 21

// LDS receive buffer
byte rxbuff[RXBUFF_SZ];
int packetchar = 0; // Incoming character.

// Display buffers and index pointers.
// Each angle's brightness is a byte that is translated to a PWM value
// via the bright2pwm lookup table.
// Two buffers implement simple double buffering, the display is one
// rotation behind the buffer being filled.
byte dispbuff0[360];
byte dispbuff1[360];
byte *fill = dispbuff0;
byte *disp = dispbuff1;
byte *dispindex = NULL; // Dummy display pointer used for synchronization.
byte *led0 = NULL;  // ledN are display pointers.
byte *led1 = NULL;
byte *led2 = NULL;
byte *led3 = NULL;
byte *buffend = NULL;
byte *buffstart = NULL;

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

#define ls 8
#define lf 10
#define rf 11
#define rs 9
int d;
int a;
short  x, y, x2, y2;
float pi180 = (pi / 180);
float tempa, tempsin, tempcos;
//////////////////
//////////////////
//////////////////
//////////////////
// Initialization
void setup() {

  tempsin = sin((4) * (pi / 180));
  tempcos = cos((4) * (pi / 180));
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  colorWipe(defaultColor, 0);
  strip.setBrightness(100);
  strip.show();
  // Initialize Serial port for LDS
  Serial1.begin(115200, SERIAL_8N1);
  // Set Bumper pins for output.
  pinMode(ls, OUTPUT);
  pinMode(lf, OUTPUT);
  pinMode(rf, OUTPUT);
  pinMode(rs, OUTPUT);
  digitalWrite(ls, HIGH);   // sets the bumper to be pressed
  delay(4);
  digitalWrite(lf, HIGH);   // sets the bumper to be pressed
  delay(4);
  digitalWrite(ls, LOW);   // sets the bumper to not pressed
  delay(4);
  digitalWrite(lf, LOW);   // sets the bumper to not pressed
  delay(4);
  digitalWrite(rf, HIGH);   // sets the bumper to be pressed
  delay(4);
  digitalWrite(rs, HIGH);   // sets the bumper to be pressed
  delay(4);
  digitalWrite(rf, LOW);   // sets the bumper to not pressed
  delay(4);
  digitalWrite(rs, LOW);   // sets the bumper to not pressed
}
// Compute checksum over receive buffer. Returns 'true' if packet is valid.
bool validpacket() {
  long chk32 = 0;
  unsigned int t  = 0;
  for (int i = 0; i < 20; i += 2) {
    t = rxbuff[i] + (unsigned int)(rxbuff[i + 1] << 8);
    chk32 = (chk32 << 1) + t;
  }
  int checksum = (chk32 & 0x7fff) + (chk32 >> 15);
  checksum = checksum & 0x7fff;
  return checksum == (rxbuff[CKL] | (rxbuff[CKH] << 8));
}


// Read one 4-degree packet from the LDS into rxbuff.
// Packet may not be valid; verify with validpacket().
void getpacket() {
  int rxp;
  bool insync = false;

  packetchar = Serial1.read();
  while (!insync) {
    while (packetchar != START_CHAR) {
      packetchar = Serial1.read();
      rxbuff[0] = packetchar;
    }
    packetchar = Serial1.read();
    while (packetchar < 0) packetchar = Serial1.read();
    rxbuff[INDEX_BYTE] = packetchar;
    insync = packetchar >= INDEX_BIAS;
  }
  //Serial.print(packetchar);
  rxp = 2;

  while (rxp < RXBUFF_SZ) {
    packetchar = Serial1.read();
    while (packetchar < 0) packetchar = Serial1.read();
    rxbuff[rxp] = packetchar;
    rxp++;
    //Serial.print(packetchar );
  }
}

int index() {
  return (rxbuff[INDEX_BYTE] - 0xA0);
}

int angle() {
  return index() << 2;
}

// returns distance in mm or a negative number on invalid data.
int dist(int n) {
  int v = n << 2;
  return (rxbuff[DATA_START + v]) | ((rxbuff[DATA_START + v + 1] & 0x9f) << 8);
}

int speed() {
  return   (rxbuff[SPEED_L] | (rxbuff[SPEED_H] << 8));
}

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

void swapbuffers() {
  noInterrupts();
  if (fill == dispbuff0) {
    fill = dispbuff1;
    dispindex = dispbuff0;
    buffstart = dispbuff0;
    buffend = dispbuff0 + 360;
  }
  else {
    fill = dispbuff0;
    dispindex = dispbuff1;
    buffstart = dispbuff1;
    buffend = dispbuff1 + 360;
  }
  interrupts();
}


void loop() {

  getpacket();

  if (validpacket()) {

    if (index() == 0) {
      swapbuffers();
    }

    a = (359 + angle() - 8) % 359;

    for (int i = 0; i < 4; ++i, ++a) {
      d = dist(i);
      tempa = a ;

      //everyting needs to be offset by 8 degrees
      x = d * cos(tempa * pi180);
      y = d * sin(tempa * pi180);
      x2 = tempcos * x - tempsin * y;
      y2 = tempsin * x + tempcos * y;

      if (y2 >= ((-1 / x) * 100) + (-168 - distance + 1) &&
          y2 <= (1 / x) * 100 + (168 + distance +2) &&
          x2 >= 150 &&
          x2 <= 250 + distance) {
        if (a > 14 && a < 29) {
          /*Left front bumper*/
          Serial.print("front left");
          Serial.println();
          //            Serial.print("angle: ");
          //            Serial.print(a);
          Serial.println();
          //            Serial.print("distance: ");
          //            Serial.print(d);
          Serial.println();
          //            Serial.print("x: ");
          //            Serial.print(x);
          Serial.println();
          //            Serial.print("y: ");
          //            Serial.print(y);
          Serial.println();
          Serial.println();
          digitalWrite(lf, HIGH);   // sets the bumper to be pressed
          delay(20);
          digitalWrite(lf, LOW);   // sets the bumper to not pressed
        } else if ( a > 32 && a <= 90 ) {
          /*Side left bumper*/
          Serial.print("side left");
          Serial.println();
          //            Serial.print("angle: ");
          //            Serial.print(a);
          Serial.println();
          //            Serial.print("distance: ");
          //            Serial.print(d);
          Serial.println();
          //            Serial.print("x: ");
          //            Serial.print(x);
          Serial.println();
          //            Serial.print("y: ");
          //            Serial.print(y);
          Serial.println();
          Serial.println();
          digitalWrite(ls, HIGH);   // sets the bumper to be pressed
          delay(20);
          digitalWrite(ls, LOW);   // sets the bumper to not pressed
        } else if (a > 323 && a < 343) {
          Serial.print("front right");
          //
          Serial.println();
          //            Serial.print("angle: ");
          //            Serial.print(a);
          Serial.println();
          //            Serial.print("distance: ");
          //            Serial.print(d);
          Serial.println();
          //            Serial.print("x: ");
          //            Serial.print(x);
          Serial.println();
          //            Serial.print("y: ");
          //            Serial.print(y);
          Serial.println();
          Serial.println();
          /*right front bumper*/
          digitalWrite(rf, HIGH);   // sets the bumper to be pressed
          delay(20);
          digitalWrite(rf, LOW);   // sets the bumper to not pressed
        } else if (a >= 270 && a < 321) {
          /*right side bumper*/
          Serial.print("right side");
          Serial.println();
          //            Serial.print("angle: ");
          //            Serial.print(a);
          Serial.println();
          //            Serial.print("distance: ");
          //            Serial.print(d);
          Serial.println();
          //            Serial.print("x: ");
          //            Serial.print(x);
          Serial.println();
          //            Serial.print("y: ");
          //            Serial.print(y);
          Serial.println();
          Serial.println();
          digitalWrite(rs, HIGH);   // sets the bumper to be pressed
          delay(20);
          digitalWrite(rs, LOW);   // sets the bumper to not pressed
        }
        else if (a >= 343 || a <= 14) {
          /*both front bumper*/
          Serial.print("both front");
          Serial.println();
          //            Serial.print("angle: ");
          //            Serial.print(a);
          Serial.println();
          //            Serial.print("distance: ");
          //            Serial.print(d);
          Serial.println();
          //            Serial.print("x: ");
          //            Serial.print(x);
          Serial.println();
          //            Serial.print("y: ");
          //            Serial.print(y);
          Serial.println();
          Serial.println();
          digitalWrite(rf, HIGH);   // sets the bumper to be pressed
          delay(20);
          digitalWrite(lf, HIGH);   // sets the bumper to be pressed
          delay(20);
          digitalWrite(rf, LOW);   // sets the bumper to not pressed
          digitalWrite(lf, LOW);   // sets the bumper to not pressed

        }
        else if (a >= 29 && a <= 32) {
          /*left corner bumpers*/
          Serial.print("corner left");
          Serial.println();
          //            Serial.print("angle: ");
          //            Serial.print(a);
          Serial.println();
          //            Serial.print("distance: ");
          //            Serial.print(d);
          Serial.println();
          //            Serial.print("x: ");
          //            Serial.print(x);
          Serial.println();
          //            Serial.print("y: ");
          //            Serial.print(y);
          Serial.println();
          Serial.println();
          digitalWrite(lf, HIGH);   // sets the bumper to be pressed
          delay(20);
          digitalWrite(ls, HIGH);   // sets the bumper to be pressed
          delay(20);
          digitalWrite(lf, LOW);   // sets the bumper to not pressed

          digitalWrite(ls, LOW);   // sets the bumper to not pressed
        }
        else if (a >= 321 && a <= 323) {
          /*right corner bumpers*/
          Serial.print("corner right");
          Serial.println();
          //            Serial.print("angle: ");
          //            Serial.print(a);
          Serial.println();
          //            Serial.print("distance: ");
          //            Serial.print(d);
          Serial.println();
          //            Serial.print("x: ");
          //            Serial.print(x);
          Serial.println();
          //            Serial.print("y: ");
          //            Serial.print(y);
          Serial.println();
          Serial.println();
          digitalWrite(rf, HIGH);   // sets the bumper to be pressed
          delay(20);
          digitalWrite(rs, HIGH);   // sets the bumper to be pressed
          delay(20);
          digitalWrite(rf, LOW);   // sets the bumper to not pressed

          digitalWrite(rs, LOW);   // sets the bumper to not pressed
        }
      }




    }
  }
}
