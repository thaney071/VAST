/*******************************************************************************

Cut down Code with out data logger
Author: Tom Haney
Date: 4 March 2015
Version 2.0

Description:
this code uses timer 1  to keep track of how
long the balloon has been in flight. Using the specified time,
the module will wait, then turn on the nichrome wire and drop the
payload. The module can also revive a interrupt on pin 5 that will 
trigger the payload drop.

*******************************************************************************/
//Used for sensor
#include <Wire.h>
#include <MS5803_I2C.h>
//micoSD card
#include <SD.h>
//Used to convert data for microSD storage
#include <stdlib.h>
//Timer interrupt
#include <TimerOne.h>

/****************************************
* 
* EDIT THESE VARIABLES TO ADJUST FLIGHT TIMES
    and control microSD
* 
****************************************/
#define TIMER_DEBUG 0 // set this variable to 1 to turn on timer 1 debugging. 0 turn off.
#define DATA_LOGGER_DEBUG 0
unsigned long cutdown_time = 360; //Cut down time. Change this variable to adjust flight duration before cutdown in seconds
const int chipSelect = 10; // this is the chip select pin for the pro mini. change this pin to configure chip select.
double base_altitude = 786.0; // Altitude of SparkFun's HQ in Boulder, CO. in (m) modify this to launch alt

/****************************************
* 
* DO NOT CHANGE THESE VARIABLES
* 
****************************************/
const int AVG_VAL = 32;
volatile int go = 0; //go stores the state of the cut down
volatile unsigned long timer_Count = 0; // keeps track of number of second since timer1 started. rolls over in 136 years.
int ledState = LOW; //Timer led on/off status. Turn on TIMER_DEBUG to use.
char temp[32] = ""; //used in converting sensor data into string.
MS5803 sensor(ADDRESS_HIGH);//specifies sensor address. This should not be change. controlled by jumper on board
// Sensor data variables. Leave these global to allow access to microSD function.
float temperature_c, temperature_f; 
double pressure_abs[AVG_VAL], pressure_relative, altitude_delta, pressure_baseline, pressure_abs_avg;


/****************************************************

This function writes data from sensor to
microSD card into separate csv files

****************************************************/

void dataLog()
{    // make a string for assembling the data to log:
    String dataString = "";

    // read three sensors and append to the string: 
    dtostrf(temperature_f,4,1, temp);
    dataString += temp;
    dataString += ",";     

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile1 = SD.open("tempf.csv", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile1)
    {
        dataFile1.println(dataString);
        dataFile1.close();
        // print to the serial port too:
        Serial.println(dataString);
    }  
    // if the file isn't open, pop up an error:
    else if(DATA_LOGGER_DEBUG)
    {
        Serial.println("error opening tempf.csv");
    } 
    else
    {
        digitalWrite(7, HIGH); // LED notifing file did not open
    }

    dataString = "";
    dtostrf(pressure_relative,4,1, temp);
    dataString += temp;

    dataString += ",";

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile2 = SD.open("presure.csv", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile2) 
    {
        dataFile2.println(dataString);
        dataFile2.close();
        // print to the serial port too:
        Serial.println(dataString);
    }  
    // if the file isn't open, pop up an error:
    else if(DATA_LOGGER_DEBUG)
    {
        Serial.println("error opening presure.csv");
    }
    else
    {
        digitalWrite(7, HIGH); // LED notifing file did not open
    }

}//END dataLog

/****************************************************

This function reads the sensor data and
stores into global variables to be
accessed by microSD function

****************************************************/

void readSensor() {

    int i = 0;
    // To measure to higher degrees of precision use the following sensor settings:
    // ADC_256 
    // ADC_512 
    // ADC_1024
    // ADC_2048
    // ADC_4096
    
    // Read temperature from the sensor in deg C. This operation takes about 
    temperature_c = sensor.getTemperature(CELSIUS, ADC_512);

    // Read temperature from the sensor in deg F. Converting
    // to Fahrenheit is not internal to the sensor.
    // Additional math is done to convert a Celsius reading.
    temperature_f = sensor.getTemperature(FAHRENHEIT, ADC_4096);

    // Read pressure from the sensor in mbar.
     for (i = 0; i < AVG_VAL-1; i++)
    {
        pressure_abs[i]= pressure_abs[i+1];

    }
        pressure_abs[AVG_VAL]= sensor.getPressure(ADC_4096);

        pressure_abs_avg = 0;

        for (i = 0; i < AVG_VAL; i++)
    {
        pressure_abs_avg= pressure_abs[i] + pressure_abs_avg;
    }
        pressure_abs_avg = pressure_abs_avg/AVG_VAL;

    // Let's do something interesting with our data.

    // Convert abs pressure with the help of altitude into relative pressure
    // This is used in Weather stations.
    pressure_relative = sealevel(pressure_abs_avg, base_altitude);

    // Taking our baseline pressure at the beginning we can find an approximate
    // change in altitude based on the differences in pressure.   
    altitude_delta = altitude(pressure_abs_avg , pressure_baseline);

    if(DATA_LOGGER_DEBUG)
    {
        // Report values via UART
        Serial.print("Temperature C = ");
        Serial.println(temperature_c);

        Serial.print("Temperature F = ");
        Serial.println(temperature_f);

        Serial.print("Pressure abs (mbar)= ");
        Serial.println(pressure_abs_avg);

        Serial.print("Pressure relative (mbar)= ");
        Serial.println(pressure_relative); 

        Serial.print("Altitude change (m) = ");
        Serial.println(altitude_delta); 
    }//END DATA_LOGGER_DEBUG
}// END readSensor
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
/***************************************
Setup - 
initialize pins, turns on interrupts needed,
sets up the microSD card and sensor.
****************************************/

void setup(void)
{   
    int i = 0;
    
        pinMode(13, OUTPUT);
     // Sets pin 8 for timer led output. uncomment for debuging timer
    pinMode(6,OUTPUT); // LED for Missing SD card
    pinMode(7,OUTPUT); // LED for file load error
    pinMode(10, OUTPUT);// This pin needs to be set as output for SPI to work.
    pinMode(9, OUTPUT); // Pin 9 controls nichrome wire
    digitalWrite(9 , LOW); // initialize pin 9 low
    Timer1.initialize(1000000); // Set timer1 interupt for every second
    Timer1.attachInterrupt(Timer1_ISR); // Attach Timer1_ISR to timer1 interupt
    attachInterrupt(0, iridium_ISR, CHANGE); //set pin 5 as interupt pin from iridium.

    if(DATA_LOGGER_DEBUG)
    {Serial.begin(9600);}//END DATA_LOGGER_DEBUG

    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {

        digitalWrite(6, HIGH);// Turns on LED notifing error

        if(DATA_LOGGER_DEBUG)
        {Serial.println("Card failed, or not present");}
        // don't do anything more:
        return;
    }

    //Retrieve calibration constants for conversion math.
    sensor.reset();
    sensor.begin();
    
    
    for (i = 0; i < AVG_VAL; i++)
    {
        pressure_abs[i]= sensor.getPressure(ADC_4096);

    }
    for (i = 0; i < AVG_VAL; i++)
    {
        pressure_abs_avg= pressure_abs[i] + pressure_abs_avg;
    }
        pressure_baseline = pressure_abs_avg/AVG_VAL;


}//END of set-up

/************************************************
* Main Program
* 
* The main program reads in data from the sensors and writes the information into csv file on the micro sd
* card. it also holds the case statement for the cut down sequence that is triggered by the timer ISR or the 
* iridium ISR. 
* 
************************************************/

void loop(void)
{    unsigned long timer_Copy;  // holds a copy of the timer_Count
    static unsigned long difference = 0; // used to calculate how long nichrome has been on.
    static unsigned long cutdown_ON = 0;// captures time cut down wire was turned on.

    readSensor();
    dataLog();

    // to read a variable which the interrupt code writes, we
    // must temporarily disable interrupts, to be sure it will
    // not change while we are reading.  To minimize the time
    // with interrupts off, just quickly make a copy, and then
    // use the copy while allowing the interrupt to keep working.

    //This case statement is the cut down sequence. 
    //Case 0 does nothing while the count down timer is running.
    //Case 1 turns on the nichrome wire and capture what time it started
    //Case 2 calculates how long the wire has been on and turns of wire once time has expired
    //Case 3 does nothing and signifies the end of the cut down sequence.
    switch (go)
    {
    case 0: 
        break;// do nothing
    case 1: 
        {
            digitalWrite(9,HIGH);
            digitalWrite(13,HIGH);
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
        digitalWrite(13,LOW);        
            }
            break;
        }
    case 3: 
        break; // do nothing
    }

    delay(250);// waits to capture data every quarter second.
}//END Main

/*********************************

iridium_ISR
Input interrupt from iridium module. When pin 5 goes high, cut down is initiated.
the timer will not set off the cut down again.

*********************************/

void iridium_ISR()
{    // Starts the cut down sequence by setting go to 1. disables interrupts very shortly to
    //prevent any other access to the go variable.
    noInterrupts();
    if( go == 0)
    {
        go  = 1;
    }
    interrupts();

}//End iridium_ISR

/************************************************
* Timer1_ISTR
* 
* the timer1 is set to interrupt every second. It will compare the number of seconds the timre has been on to 
* the desired flight duration. when the timer exceeds the flight duration, go will be set to the next state and
* start the cut down sequence. 
************************************************/

void Timer1_ISR(void)
{    // TIMER_DEBUG keeps track of on/off stat of led
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

//END of file
