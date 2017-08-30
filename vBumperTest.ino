const int distance = 10;
const float distanceFromCenterOfRotationTolazer = 2.5;
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

// Time to display one degree, in microseconds = 200000 / 360
#define ONE_DEGREE 556

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
byte *buffend = NULL;
byte *buffstart = NULL;

//left side, left front, right front , right side bumpers
int ls = 8;
int lf = 10;
int rf = 11;
int rs = 9;

//////////////////
// Initialization
void setup() {
  // Initialize Serial port for LDS
  Serial1.begin(115200, SERIAL_8N1);
  // Set Bumper pins for output.
  pinMode(ls, OUTPUT);
  pinMode(lf, OUTPUT);
  pinMode(rf, OUTPUT);
  pinMode(rs, OUTPUT);
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

int d;
int a;
float  x, y, x2, y2;
float pi180 = (180 / pi);
float tempa, tempsin, tempcos, tempCorrection;
void loop() {

  getpacket();

  if (index() == 0) {
    swapbuffers();
    //      Serial.println(bright2pwm[*(led0)]);
  }

  a = angle();
  for (int i = 0; i < 4; ++i, ++a) {
    d = dist(i);
    tempCorrection = asin(distanceFromCenterOfRotationTolazer / d) * 180 / pi;

    tempa = fmod(a + tempCorrection,359);

    x = d * cos(tempa * pi180);
    y = d * sin(tempa * pi180);
    //Rotate everything about the origin by 8 degrees
    tempsin = sin((360 - 4) * pi180);
    tempcos = cos((360 - 4) * pi180);
    x2 = tempcos * x - tempsin * y;
    y2 = tempsin * x + tempcos * y;
    x = x2;
    y = y2;

    if (y >= ((-1 / x) * 100) + (-168 - distance - .5 ) && y <= (1 / x) * 100 + (168 + distance + distance - .25)) {
      if (x >= 160 && x <= 250 + distance) {
        if (a > 6 && a < 25) {
          /*Left front bumper*/
          Serial.print("front left");
          Serial.println();
          Serial.print("tempCorrection: ");
          Serial.print(tempCorrection);
          Serial.println();
          Serial.print("angle: ");
          Serial.print(tempa);
          Serial.println();
          Serial.print("distance: ");
          Serial.print(d);
          Serial.println();
          Serial.print("x: ");
          Serial.print(x);
          Serial.println();
          Serial.print("y: ");
          Serial.print(y);
          Serial.println();
          Serial.println();
          digitalWrite(lf, HIGH);   // sets the bumper to be pressed
          delay(10);
          digitalWrite(lf, LOW);   // sets the bumper to not pressed
        } else if ( a > 34 && a <= 90 ) {
          /*Side left bumper*/
          Serial.print("side left");
          Serial.println();
          Serial.print("tempCorrection: ");
          Serial.print(tempCorrection);
          Serial.println();
          Serial.print("angle: ");
          Serial.print(tempa);
          Serial.println();
          Serial.print("distance: ");
          Serial.print(d);
          Serial.println();
          Serial.print("x: ");
          Serial.print(x);
          Serial.println();
          Serial.print("y: ");
          Serial.print(y);
          Serial.println();
          Serial.println();
          digitalWrite(ls, HIGH);   // sets the bumper to be pressed
          delay(10);
          digitalWrite(ls, LOW);   // sets the bumper to not pressed
        } else if (a > 335 && a < 354) {
          Serial.print("front right");

          Serial.println();
          Serial.print("tempCorrection: ");
          Serial.print(tempCorrection);
          Serial.println();
          Serial.print("angle: ");
          Serial.print(tempa);
          Serial.println();
          Serial.print("distance: ");
          Serial.print(d);
          Serial.println();
          Serial.print("x: ");
          Serial.print(x);
          Serial.println();
          Serial.print("y: ");
          Serial.print(y);
          Serial.println();
          Serial.println();
          /*right front bumper*/
          digitalWrite(rf, HIGH);   // sets the bumper to be pressed
          delay(10);
          digitalWrite(rf, LOW);   // sets the bumper to not pressed
        } else if (a >= 270 && a <= 330) {
          /*right side bumper*/
          Serial.print("right side");

          Serial.println();
          Serial.print("tempCorrection: ");
          Serial.print(tempCorrection);
          Serial.println();
          Serial.print("angle: ");
          Serial.print(tempa);
          Serial.println();
          Serial.print("distance: ");
          Serial.print(d);
          Serial.println();
          Serial.print("x: ");
          Serial.print(x);
          Serial.println();
          Serial.print("y: ");
          Serial.print(y);
          Serial.println();
          Serial.println();
          digitalWrite(rs, HIGH);   // sets the bumper to be pressed
          delay(10);
          digitalWrite(rs, LOW);   // sets the bumper to not pressed
        }
        else if (a >= 354 || a <= 6) {
          /*both front bumper*/
          Serial.print("both front");
          Serial.println();
          Serial.print("tempCorrection: ");
          Serial.print(tempCorrection);
          Serial.println();
          Serial.print("angle: ");
          Serial.print(tempa);
          Serial.println();
          Serial.print("distance: ");
          Serial.print(d);
          Serial.println();
          Serial.print("x: ");
          Serial.print(x);
          Serial.println();
          Serial.print("y: ");
          Serial.print(y);
          Serial.println();
          Serial.println();
          digitalWrite(rf, HIGH);   // sets the bumper to be pressed
          delay(100);
          digitalWrite(lf, HIGH);   // sets the bumper to be pressed
          delay(100);
          digitalWrite(rf, LOW);   // sets the bumper to not pressed
          delay(10);
          digitalWrite(lf, LOW);   // sets the bumper to not pressed

          delay(10);

        }
        else if (a >= 25 && a <= 34) {
          /*left corner bumpers*/
          Serial.print("corner left");
          Serial.println();

          Serial.print("angle: ");
          Serial.print(tempa);
          Serial.println();
          Serial.print("distance: ");
          Serial.print(d);
          Serial.println();
          Serial.print("x: ");
          Serial.print(x);
          Serial.println();
          Serial.print("y: ");
          Serial.print(y);
          Serial.println();
          Serial.println();
          digitalWrite(lf, HIGH);   // sets the bumper to be pressed
          delay(10);
          digitalWrite(ls, HIGH);   // sets the bumper to be pressed
          delay(10);
          digitalWrite(lf, LOW);   // sets the bumper to not pressed
          delay(10);
          digitalWrite(ls, LOW);   // sets the bumper to not pressed

          delay(10);
        }
        else if (a > 330 && a <= 335) {
          /*right corner bumpers*/
          Serial.print("corner right");
          Serial.println();
          Serial.print("angle: ");
          Serial.print(tempa);
          Serial.println();
          Serial.print("distance: ");
          Serial.print(d);
          Serial.println();
          Serial.print("x: ");
          Serial.print(x);
          Serial.println();
          Serial.print("y: ");
          Serial.print(y);
          Serial.println();
          Serial.println();
          digitalWrite(rf, HIGH);   // sets the bumper to be pressed
          delay(10);
          digitalWrite(rs, HIGH);   // sets the bumper to be pressed
          delay(10);
          digitalWrite(rf, LOW);   // sets the bumper to not pressed
          delay(10);
          digitalWrite(rs, LOW);   // sets the bumper to not pressed
          delay(10);


        }
      }
    }
  }
}
