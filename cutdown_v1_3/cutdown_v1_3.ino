#include <TimerOne.h>
#include <stdlib.h>



/****************************************
 * 
 * EDIT THESE VARIABLES TO ADJUST FLIGHT TIMES
 * 
 ****************************************/
#define TIMER_DEBUG 1 // set this variabel to 1 to turn on timer 1 debuging. 0 turn off.
unsigned long cutdown_time = 10; //Cutdown time. Change this variable to adjust flight duration before cutdown in seconds



/****************************************
 * 
 * DO NOT CHANGE THESE VARIABLES
 * 
 ****************************************/
volatile int go = 0; //go stores the state of the cutdown
volatile unsigned long timer_Count = 0; // keeps track of number of second since timer1 started. rolls over in 136 years.


int ledState = LOW; //Timer led on/off status. Turn on TIMER_DEBUG to use.

/***************************************
  Setup - 
  initalizes pins, turns on interupts needed,
  sets up the microSD card and senor.


****************************************/
void setup(void)
{
  if ( TIMER_DEBUG )
  {
    pinMode(8, OUTPUT);
  } // Sets pin 8 for timer led output. uncomment for debuging timer

  pinMode(9, OUTPUT); // Pin 9 controls nichrome wire
  digitalWrite(9 , LOW); // initialize pin 9 low
  Timer1.initialize(1000000); // Set timer1 interupt for every second
  Timer1.attachInterrupt(Timer1_ISR); // Attach Timer1_ISR to timer1 interupt
  attachInterrupt(0, iridium_ISR, CHANGE); //set pin 5 as interupt pin from iridium.
  Serial.begin(9600);
}//END of setup


/************************************************
 * Timer1_ISTR
 * 
 * the timer1 is set to interupt every second. It will compare the number of seconds the timre has been on to 
 * the desired flight duration. when the timer exceds the flight duration, go will be set to the next state and
 * start the cutdown sequence. 
 ************************************************/

void Timer1_ISR(void)
{
  // TIMER_DEBUG keeps track of on/off stat of led
  if(TIMER_DEBUG){
    if (ledState == LOW) {
      ledState = HIGH;

    }
    else {
      ledState = LOW;
    }
    digitalWrite(8, ledState);
  }


  timer_Count = timer_Count + 1;  // increase when  interupt occurs

  //Once timer has reach flight time, trigger cutdown in main.
  if((timer_Count >= cutdown_time) && (go == 0)) 
  {
    go = 1;
  }
}//END Timer1_ISR


/************************************************
 * Main Program
 * 
 * The main program reades in data from the sensors and writes the infromation into csv file on the micro sd
 * card. it also holds the case statement for the cutdown sequence that is trigered by the timer ISR or the 
 * iridium ISR. 
 * 
 ************************************************/
void loop(void)
{
  unsigned long timer_Copy;  // holds a copy of the timer_Count
  static unsigned long difference = 0; // used to calculate how long nichrome has been on.
  static unsigned long cutdown_ON = 0;// captures time cutdown wire was turned on.
  
  // to read a variable which the interrupt code writes, we
  // must temporarily disable interrupts, to be sure it will
  // not change while we are reading.  To minimize the time
  // with interrupts off, just quickly make a copy, and then
  // use the copy while allowing the interrupt to keep working.


  //This case statement is the cutdown sequence. 
  //Case 0 does nothing while the coutdown timer is running.
  //Case 1 turns on the nichrome wire and capture what time it started
  //Case 2 calculates how long the wire has been on and turns of wire once time has expired
  //Case 3 does nothing and signifies the end of the cutdown sequence.
  switch (go)
  {
  case 0: 
    break;// do nothing
  case 1: 
    {
      digitalWrite(9,HIGH);
      noInterrupts();
      go  = 2;
      cutdown_ON = timer_Count;
      interrupts();       
      break;
    }  
  case 2:
    {
      noInterrupts();
      difference = timer_Count - cutdown_ON;
      interrupts();
      if( difference >= 10)
      {
        noInterrupts();
        go  = 3;
        interrupts();
        digitalWrite(9,LOW);        
      }
      break;
    }
  case 3: 
    break; // do nothing
  }


  delay(100);
}//END Main

/*********************************

  iridium_ISR
  Input interupt from iridium module. When pin 5 goes high, cutdown is initiated.
  the timer will not set off the cutdown again.

*********************************/

void iridium_ISR()
{
  // Startes the cutdown sequence by seting go to 1. disables interupts very shortly to
  //prevent any other access to the go variable.
  noInterrupts();
  if( go == 0)
  {
  go  = 1;
  }
  interrupts();

}//End iridium_ISR
