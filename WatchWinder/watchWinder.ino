////////////////////////////////////////////////////////////////////
// This Arduino sketch uses a 28BYJ-48, using a ULN2003 interface 
// board to drive the stepper for use in an automatic watch winder
// Application.
//
// To increase or increase the delay between full revolutions change 
// Time between variable by turning the knob. Change the revolutions variable to change the number of 
// full revolutions betweens pauses turn the knob.
//
// The bulk of this sketch was taken from http://www.4tronix.co.uk/arduino/Stepper-Motors.php (Thanks guys)
// The 28BYJ-48 motor is a 4-phase, 8-beat motor, geared down by
// a factor of 68. One bipolar winding is on motor pins 1 & 3 and
// the other on motor pins 2 & 4. The step angle is 5.625/64 and the
// operating Frequency is 100pps. Current draw is 92mA.
////////////////////////////////////////////////////////////////////

// The Rolex Watch winder turns 1 turn every 1.4 minutes for a total of approximatly 500 turns per day in each direction 

#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);

char str[10];

//declare variables for the motor pins
int motorPin1 = 8;    // Blue   - 28BYJ48 pin 1
int motorPin2 = 9;    // Pink   - 28BYJ48 pin 2
int motorPin3 = 10;    // Yellow - 28BYJ48 pin 3
int motorPin4 = 11;    // Orange - 28BYJ48 pin 4
                        // Red    - 28BYJ48 pin 5 (VCC)

int motorSpeed = 1200;  //variable to set stepper speed
int count = 0;          // count of steps made
int countsperrev = 512; // number of steps per full revolution
float revs = 1; // number of revolutions
int lookup[8] = {B01000, B01100, B00100, B00110, B00010, B00011, B00001, B01001};
long icount = 0;
float minWait = 1.44; // Minutes to wait between each revolution

float potPin = 2;
float potPin2 = 3;
int buttonPin = 4;
float button = 0.00;

int ledPin = 5;
int val = 0;

//////////////////////////////////////////////////////////////////////////////
void setup() {
  //declare the motor pins as outputs
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(ledPin, HIGH);
  Serial.begin(9600);
  while(count <= (countsperrev*revs)){
      clockwise();
      count++;
      if(count > (countsperrev*revs)){
        count = 0;
        break;
      }
    }
}

//////////////////////////////////////////////////////////////////////////////
void loop(){
  float sum = 0;
  for (int i=0; i< 32; i++){ //1500
    sum += ((analogRead(potPin))/95.00);
  }
  float val = sum / 32;
  float sum2 = 0;
  for (int i=0; i< 32; i++){ //512
    sum2 += (round((analogRead(potPin2))/95));
  }
  int val2 = sum2 / 32;
  minWait = (round(val*10)/10.0);
  long waitBetween = round(minWait*377); // Delay between turns minWait * 60000 (60000 = 1 Minute)
  revs = (val2+1);
  if(icount == waitBetween){
    while(count <= (countsperrev*revs)){
     clockwise();
      count++;
      if(count > (countsperrev*revs)){
        count = 0;
        break;
      }
    }
  }
  //delay(waitBetween);
  if(icount == (waitBetween*2)){
    while(count <= (countsperrev*revs)){
      anticlockwise();
      count++;
      if(count > (countsperrev*revs)){
        count = 0;
        break;
      }
    }
  }
  //delay(waitBetween);
  Serial.println(icount);
  icount++;
  if(icount >= ((waitBetween*2)+1)){
    icount=0;
  }
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_helvB08);

    u8g.drawStr(30, 10, "Watch Winder");
    
    u8g.drawStr( 0, 35, "Time Between:");
    u8g.drawStr( 80, 35, dtostrf(minWait, 5, 2, str));
    //u8g.drawStr( 120, 35, " Mins");
    
    u8g.drawStr( 0, 50, "Revolutions:");
    u8g.drawStr( 80, 50, dtostrf(revs, 5, 2, str));
    //u8g.drawStr( 120, 50, "Turns");
    
    //u8g.drawStr( 80, 45, dtostrf(f, 5, 2, str));
    //u8g.drawStr( 120, 45, "\260F");
    
    //u8g.drawStr( 0, 60, "Heat index:");
    //u8g.drawStr( 80, 60, dtostrf(hi, 5, 2, str));
    //u8g.drawStr( 120, 60, "\260F");
    
  } while( u8g.nextPage() );
}

//////////////////////////////////////////////////////////////////////////////
//set pins to ULN2003 high in sequence from 1 to 4
//delay "motorSpeed" between each pin setting (to determine speed)
void anticlockwise()
{
  for(int i = 0; i < 8; i++)
  {
    setOutput(i);
    delayMicroseconds(motorSpeed);
  }
}

void clockwise()
{
  for(int i = 7; i >= 0; i--)
  {
    setOutput(i);
    delayMicroseconds(motorSpeed);
  }
}

void setOutput(int out)
{
  digitalWrite(motorPin1, bitRead(lookup[out], 0));
  digitalWrite(motorPin2, bitRead(lookup[out], 1));
  digitalWrite(motorPin3, bitRead(lookup[out], 2));
  digitalWrite(motorPin4, bitRead(lookup[out], 3));
}
