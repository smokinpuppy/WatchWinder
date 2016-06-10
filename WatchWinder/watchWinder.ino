////////////////////////////////////////////////////////////////////
// This Arduino sketch uses a 28BYJ-48, using a ULN2003 interface 
// board to drive the stepper for use in an automatic watch winder
// Application.
//
// This winder has a lock button. To change settings from the controls
// first press the lock button to enter edit mode (You'll see "Unlocked" 
// on the bottom right corner), once your settings have been adjusted, 
// press the lock button again to lock (You'll see "Locked" reappear).
// To increase or increase the delay between full revolutions change 
// Time between variable by turning the knob 1. This setting is in Seconds. 
// To change the revolutions variable to change the number of 
// full revolutions betweens pauses turn the knob 2.
//
// The motor control for this sketch was mostly taken from:
// http://www.4tronix.co.uk/arduino/Stepper-Motors.php (Thanks guys)
// The 28BYJ-48 motor is a 4-phase, 8-beat motor, geared down by
// a factor of 68. One bipolar winding is on motor pins 1 & 3 and
// the other on motor pins 2 & 4. The step angle is 5.625/64 and the
// operating Frequency is 100pps. Current draw is 92mA.
////////////////////////////////////////////////////////////////////

// The Rolex Watch winder turns 1 turn every 86.4 Seconds for a total of 
// approximatly 500 turns per day in each direction 

#include "U8glib.h" // include OLED Driver
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0); // Initiate OLED object

// Declare OLED Variables
char str[10];

//declare variables for the motor pins
int motorPin1 = 8;    // Blue   - 28BYJ48 pin 1
int motorPin2 = 9;    // Pink   - 28BYJ48 pin 2
int motorPin3 = 10;    // Yellow - 28BYJ48 pin 3
int motorPin4 = 11;    // Orange - 28BYJ48 pin 4
                        // Red    - 28BYJ48 pin 5 (VCC)

// Declare Motor Variables
int motorSpeed = 1200;  //variable to set stepper speed
int count = 0;          // count of steps made
int countsperrev = 512; // number of steps per full revolution
float revs = 1; // number of revolutions
int lookup[8] = {B01000, B01100, B00100, B00110, B00010, B00011, B00001, B01001};

// Declare Timing Variables
long icount = 0;   // Loop Counter
long minWait = 86; // Seconds to wait between each revolution
float loopsPerSec = 6.81; // # of Loops per second. 6.81 Loops = 1 Second
int longPress_secs = 3;
int LONGPRESS_LEN= (loopsPerSec*longPress_secs);

// Declare Other Hardware Variables
int potPin = 2;
int potPin2 = 3;
int ledPin = 5;
int val = 0;

// Declare Button Varialbes
int buttonPin = 4;
boolean locked = true;
enum { EV_NONE=0, EV_SHORTPRESS, EV_LONG };
boolean button_was_pressed; // previous button state
int button_pressed_counter; // Press running duration

// Delclare Switch Variables
int switchPin = 3;
boolean bidirectional = true; // Turns both clockwise and aniclockwise if true.

//////////////////////////////////////////////////////////////////////////////
void setup() {
  //declare the motor pins as outputs
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);

  // Declare Other Hardware pins and modes
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  pinMode(switchPin, INPUT);
  digitalWrite(switchPin, HIGH);
  button_was_pressed = false;
  button_pressed_counter = 0;
  digitalWrite(ledPin, HIGH); // Turn on Power indicator LED
  pinMode(potPin,INPUT);
  Serial.begin(9600);
  digitalWrite(ledPin, LOW); // Turn off Power indicator LED while motor is running

  // Turn 1 full revolution to lock motor
  while(count <= (countsperrev*revs)){
      clockwise();
      count++;
      if(count > (countsperrev*revs)){
        count = 0;
        digitalWrite(ledPin, HIGH); // Turn on Power indicator LED after motor finishes running
        break;
      }
    }
}
//////////////////////////////////////////////////////////////////////////////

// Function to handle button press
int handle_button(){
  int event;
  int button_now_pressed = !digitalRead(buttonPin); // pin low = pressed

  if(!button_now_pressed && button_was_pressed){
    if(button_pressed_counter < LONGPRESS_LEN){
      event = 1;
    } else {
      event = 2;
    }
  } else {
    event = 0;
  }

  if(button_now_pressed){
    ++button_pressed_counter;
  } else {
    button_pressed_counter = 0;
  }

  button_was_pressed = button_now_pressed;
  return event;
}

//////////////////////////////////////////////////////////////////////////////
void loop(){
  bidirectional = digitalRead(switchPin);
  // Check if lock button is pushed for longPresSecs and set locked flag accordingly
  int event = handle_button();
  if(event == 2){
      if(locked){
        locked = false;
      } else if(!locked){
        locked = true;
      }
  }

  // Read Potentiometer output and adjust for values if locked flag is false
  if(!locked){
    //WaitBetween Selector
    long sum = 0;
    for (uint8_t i=0; i< 128; i++){ //1500
      sum += (round((analogRead(potPin)*2)/10.00));
    }
    long val = sum / 128;

    // Revolutions Selector
    float sum2 = 0;
    for (int i=0; i< 32; i++){ //512
      sum2 += (round((analogRead(potPin2))/95));
    }
    int val2 = sum2 / 32;

    // Set WaitBetween and Revolutions variables
    minWait = (round(val*100)/10)/10;
    revs = (val2+1);
  }
  
  long waitBetween = minWait*loopsPerSec; // Delay between turns minWait * loopsPerSec

  // If WaitBetween is reached make one revolution clockwise
  if(icount == waitBetween){
    digitalWrite(ledPin, LOW);
    while(count <= (countsperrev*revs)){
      clockwise();
      count++;
      if(count > (countsperrev*revs)){
        count = 0;
        digitalWrite(ledPin, HIGH);
        if(!bidirectional){
          icount = 0;
        }
        break;
      }
    }
  }

  // If WaitBetween is reached make one revolution "Clockwise"
  if(icount == (waitBetween*2)){
    digitalWrite(ledPin, LOW);
    while(count <= (countsperrev*revs)){
      anticlockwise();
      count++;
      if(count > (countsperrev*revs)){
        count = 0;
        digitalWrite(ledPin, HIGH);
        break;
      }
    }
  }

  Serial.println(icount); // Print loop count to Serial for calibration
  icount++;               // Increment loop count

  // Reset loop counter when WaitBetween is doubled plus 1
  if(icount >= ((waitBetween*2)+1)){
    icount=0;
  }

  // Begin display output
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_helvB08); // Set OLED font and size

    u8g.drawStr(30, 10, "Watch Winder");  // Header
    
    u8g.drawStr( 0, 32, "Time Between:");               // WaitBetween Display heading
    u8g.drawStr( 80, 32, dtostrf(minWait, 5, 0, str));  // WaitBetween Setting
    u8g.drawStr( 110, 32, "S");                         // WaitBetween Unit (Seconds)
    
    u8g.drawStr( 0, 47, "Revolutions:");                // Revolutions Display heading
    u8g.drawStr( 80, 47, dtostrf(revs, 5, 0, str));     // Revolutions Setting
    u8g.drawStr( 110, 47, "T");                         // Revolutions Unit (Turns)

    // Display rotation setting [Bottom Left of Screen]
    if (bidirectional){
      u8g.drawStr( 0, 62, "Bidirectional"); // Turning both clockwise and anticlockwise
    } else {
      u8g.drawStr( 0, 62, "Clockwise"); // Only turning Clockwise
    }

    // Display Lock state [Bottom Right of Screen]
    if (locked){
      u8g.drawStr( 90, 62, "Locked");
    } else {
      u8g.drawStr( 80, 62, "Unlocked");
    }
    
  } while( u8g.nextPage() );
}

//////////////////////////////////////////////////////////////////////////////
//set pins to ULN2003 high in sequence from 1 to 4
//delay "motorSpeed" between each pin setting (to determine speed)

// Clockwise Function
void clockwise()
{
  for(int i = 0; i < 8; i++)
  {
    setOutput(i);
    delayMicroseconds(motorSpeed);
  }
}

// Anticlockwise Function
void anticlockwise()
{
  for(int i = 7; i >= 0; i--)
  {
    setOutput(i);
    delayMicroseconds(motorSpeed);
  }
}

// Lookup function for stepper motor
void setOutput(int out)
{
  digitalWrite(motorPin1, bitRead(lookup[out], 0));
  digitalWrite(motorPin2, bitRead(lookup[out], 1));
  digitalWrite(motorPin3, bitRead(lookup[out], 2));
  digitalWrite(motorPin4, bitRead(lookup[out], 3));
}
