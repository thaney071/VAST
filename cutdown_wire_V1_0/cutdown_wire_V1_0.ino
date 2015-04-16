/* cutdown_wire_V1.0.ino
  
   Tom Haney
   VAST
   12 November 2014
   thaney@vandals.uidaho.edu
   
  
   program for arduino pro mini. This program waits a set time before setting the output pin
   high to turn on the cutdown wire system. It can be preempted via an input on intrupt0 on
   pin 2 of the pro mini microcontroller.
 *---------------------------------------------------------------------
 */

// Declares what pin to use as output. change this value to to change what output pin is used.
int output_pin = 9;

long int seconds = 1000;
long int minutes = 60;
long int FlightTime = 1 * seconds; //Put number of minutes for flight before cutdown occurs.

void setup()
{    
  pinMode(output_pin , OUTPUT); // Sets pin 9 to the nichrome as output.
  digitalWrite(output_pin, LOW); // initializes pin 9 to low.
  
  // set pin 2 as an interupt pin on the rising edge and executes the 
  // cutdown function
  
  attachInterrupt(0, cutdown, RISING); //set pin 5 as interupt pin from iridium.
  
  
  
}

void loop()
{  
  //digitalWrite(7,HIGH);
  while( millis()  < FlightTime ); // millis() returns the number of milliseconds the program has be running.  

  //Executes cutdown sequence.
  cutdown();

  //Stops program from starting over when finished.
  while(1); 
}

/*******************************************************************
 * NAME :            cutdown()
 *
 * DESCRIPTION :     Sets output pin to HIGH (3.3V) for 10 seconds then back to LOW (0V)
 *
 *
 * CHANGES :
 * DATE		    		WHO     			DETAIL
 * 12.Nov.2014      	Tom Haney			Orginal
 *       
 ********************************************************************/

void cutdown()
{
  // Outputs 3.3 V signal to mosfet.
  digitalWrite(output_pin, HIGH);
  digitalWrite(13, HIGH); //led indicats nichrome should be on.
  
  //druation for cutdown system to be on set to 10 seconds
  long int time = millis();  
  
  while((millis() - time)/1000 < 10 );  

   //signal for mosfet to turn off is set to 0V
  digitalWrite(output_pin, LOW);
  digitalWrite(13, LOW);
}
