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


void setup()
{
  
  //sets output_pin as an output pin
  pinMode(output_pin , OUTPUT);
  digitalWrite(output_pin, LOW);
  // set pin 2 as an interupt pin on the rising edge and executes the 
  // cutdown function
  attachInterrupt(0, cutdown, RISING);
}

void loop()
{
  int time = millis();
  //time before cutdown
  while(( millis() - time)/1000 < 5 );
  //delay(5000); 

  //Executes cutdown sequence.
  cutdown();

  //Stops program from starting over when finished.
  while(1); 
}

/*******************************************************************
 * NAME :            cutdown()
 *
 * DESCRIPTION :     Sets output pin to HIGH (5V) for 10 seconds then back to LOW (0V)
 *
 *
 * CHANGES :
 * DATE		    		WHO     			DETAIL
 * 12.Nov.2014      	Tom Haney			Orginal
 *       
 */

void cutdown()
{
  // signal for mosfet to turn on is set to 5V
  digitalWrite(output_pin, HIGH);
  digitalWrite(13, HIGH);
  //druation for cutdown system to be on set to 10 seconds
  int time = millis();
  //time before cutdown
  while((millis() - time)/1000 < 15 );
  //delay(10000);

  // signal for mosfet to turn off is set to 0V
  digitalWrite(output_pin, LOW);
  digitalWrite(13, LOW);
}
