//Used for sensor
#include <Wire.h>
#include <MS5803_I2C.h>

/****************************************
* 
* EDIT THESE VARIABLES TO ADJUST FLIGHT TIMES
    
* 
****************************************/
const unsigned long int MAX_TIME = 10000; //4800000; // number of second of flight set for 1 hr 20 min
double MAX_ALTITUDE = 21336;// set to 70k ft Max hight before cutdown will occure (75k ft) 22860 meters

double base_altitude = 750; // Altitude of launch location in (m) altituted of washtucna

/****************************************
* 
* DO NOT CHANGE THESE VARIABLES
* 
****************************************/
const int AVG_VAL = 32;
MS5803 sensor(ADDRESS_HIGH);//specifies sensor address. This should not be change. controlled by jumper on board
double pressure_abs[AVG_VAL], pressure_relative, altitude_delta, pressure_baseline, cur_altitude, pressure_abs_avg;
#define GREEN 10
#define RED 11
#define BUTTON 12
volatile uint8_t active = 0; // flag to activate cutdown


/*************************************************

Thanks to Mike Grusin for letting me borrow the functions below from 
the BMP180 example code. (From arduino library)

*************************************************/

double sealevel(double P, double A)
// Given a pressure P (mbar) taken at a specific altitude (meters),
// return the equivalent pressure (mbar) at sea level.
// This produces pressure readings that can be used for weather measurements.
{    return(P/pow(1-(A/44330.0),5.255));
}double altitude(double P, double P0)
// Given a pressure measurement P (mbar) and the pressure at a baseline P0 (mbar),
// return altitude (meters) above baseline.
{    return(44330.0*(1-pow(P/P0,1/5.255)));
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(9,OUTPUT);
  pinMode(2,INPUT);
  pinMode(BUTTON,INPUT_PULLUP);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  digitalWrite(RED, HIGH);
  
  
  int i = 0;
  //Retrieve calibration constants for conversion math.
    sensor.reset();
    sensor.begin();
    cur_altitude = base_altitude;
   for (i = 0; i < AVG_VAL; i++)
    {
        pressure_abs[i]= sensor.getPressure(ADC_4096); //sensor.getPressure(ADC_4096);

    }
    for (i = 0; i < AVG_VAL; i++)
    {
        pressure_abs_avg= pressure_abs[i] + pressure_abs_avg;
    }
        pressure_baseline = pressure_abs_avg/AVG_VAL; 

}// END SETUP

void loop() {
  
  static unsigned long int start_time = millis();
  int i =0;
  static int system_on = 0;
  long unsigned int button_time = 0;
  while(!system_on)
  {
    while(digitalRead(BUTTON));
    button_time = millis();
    digitalWrite(GREEN,HIGH);
    while(!digitalRead(BUTTON));
    if( (millis() - button_time) > 1000)
    {
      digitalWrite(RED,LOW);
      system_on = 1;
    }
    else 
    {
     digitalWrite(GREEN, LOW); 
    }
    
  }//end if system_on
  
 
  // Read pressure from the sensor in mbar.
     for (i = 0; i < AVG_VAL-1; i++)
    {
        pressure_abs[i]= pressure_abs[i+1];

    }
        pressure_abs[AVG_VAL-1]= sensor.getPressure(ADC_4096);

        pressure_abs_avg = 0;

        for (i = 0; i < AVG_VAL; i++)
    {
        pressure_abs_avg= pressure_abs[i] + pressure_abs_avg;
    }
        pressure_abs_avg = pressure_abs_avg/AVG_VAL;
  
  altitude_delta = altitude( pressure_abs_avg, pressure_baseline);
  cur_altitude = base_altitude + altitude_delta;
  
 if( cur_altitude > MAX_ALTITUDE )
  {
    active = 1;
  }//MAX_ALTITUDE
  Serial.print("Current Altitude : ");
  Serial.println(cur_altitude);
  Serial.print("Altitude delta : ");
  Serial.println(altitude_delta);
  Serial.print("Pressure abs : ");
  Serial.println(pressure_abs_avg);
 
  
  //Timer 
  Serial.print("Time remaining: ");
  Serial.println(MAX_TIME-(millis() - start_time));
  
  if ( (millis() - start_time) >= MAX_TIME ) 
  {
    active = 1;
  }//end IF time control
  
  //Iridium input
  if(digitalRead(2))
  {
    active = 1;
  }//end if read pin 2
  
  //Cutdown activate
  if(active)
  {
    cutdown();
    //system_on = 0;
    //active = 0;
    digitalWrite(10,LOW);
    digitalWrite(11,HIGH);
    while(1);
  }//end if
  
  //delay(1000);
  
}// END LOOP

void cutdown()
{
//   digitalWrite(9,HIGH);
//    delay(10000); //wait 10 seconds for cutdown
//    digitalWrite(9,LOW);
    
    analogWrite(9,24);
    delay(5000); //wait 10 seconds for cutdown
    digitalWrite(9,LOW);
  
}
