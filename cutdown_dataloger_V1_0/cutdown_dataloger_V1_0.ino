#include <Wire.h>
#include <MS5803_I2C.h>
#include <SD.h>
#include <stdlib.h>
#include <TimerOne.h>

// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// Teensy 2.0: pin 0
// Teensy++ 2.0: pin 20
const int chipSelect = 10;
const int led = LED_BUILTIN;  // the pin with a LED
char temp[32] = "";
int output_pin = 9;

long int seconds = 1000;
long int minutes = 60;
long int FlightTime = 1 * seconds * minutes; //Put number of minutes for flight before cutdown occurs.
long int count = 0;

// Begin class with selected address
// available addresses (selected by jumper on board) 
// default is ADDRESS_HIGH

//  ADDRESS_HIGH = 0x76
//  ADDRESS_LOW  = 0x77

MS5803 sensor(ADDRESS_HIGH);

//Create variables to store results
float temperature_c, temperature_f;
double pressure_abs, pressure_relative, altitude_delta, pressure_baseline;

// Create Variable to store altitude in (m) for calculations;
double base_altitude = 786.0; // Altitude of SparkFun's HQ in Boulder, CO. in (m)

void setup()
{
  pinMode(led, OUTPUT);
  Timer1.initialize(1);
  Timer1.attachInterrupt(Count_Time); // blinkLED to run every 0.15 seconds
  
  pinMode(output_pin , OUTPUT); // Sets pin 9 to the nichrome as output.
  digitalWrite(output_pin, LOW); // initializes pin 9 to low.
  
  // set pin 2 as an interupt pin on the rising edge and executes the 
  // cutdown function
  
  attachInterrupt(0, cutdown, RISING); //set pin 5 as interupt pin from iridium.
  
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
   }
   
   Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  
  Serial.println("card initialized.");
   
    //Retrieve calibration constants for conversion math.
    sensor.reset();
    sensor.begin();
    
    pressure_baseline = sensor.getPressure(ADC_4096);

}

void loop() {
  
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
  temperature_f = sensor.getTemperature(FAHRENHEIT, ADC_512);
  
  // Read pressure from the sensor in mbar.
  pressure_abs = sensor.getPressure(ADC_4096);
  
  // Let's do something interesting with our data.
  
  // Convert abs pressure with the help of altitude into relative pressure
  // This is used in Weather stations.
  pressure_relative = sealevel(pressure_abs, base_altitude);
  
  // Taking our baseline pressure at the beginning we can find an approximate
  // change in altitude based on the differences in pressure.   
  altitude_delta = altitude(pressure_abs , pressure_baseline);
  
  // Report values via UART
  //Serial.print("Temperature C = ");
 // Serial.println(temperature_c);
  
  Serial.print("Temperature F = ");
  Serial.println(temperature_f);
  
  //Serial.print("Pressure abs (mbar)= ");
  //Serial.println(pressure_abs);
   
  Serial.print("Pressure relative (mbar)= ");
  Serial.println(pressure_relative); 
  
  Serial.print("Altitude change (m) = ");
  Serial.println(altitude_delta); 


  
// make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
 
 dtostrf(temperature_f,4,3, temp);
  
    dataString += temp;
   
      dataString += ",";     

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile1 = SD.open("tempf.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile1) {
    dataFile1.println(dataString);
    dataFile1.close();
    // print to the serial port too:
    Serial.println(dataString);
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening tempf.txt");
  } 
 
 
  dataString = "";
  dtostrf(pressure_relative,4,3, temp);
      dataString += temp;
   
      dataString += ",";
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile2 = SD.open("presure.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile2) {
    dataFile2.println(dataString);
    dataFile2.close();
    // print to the serial port too:
    Serial.println(dataString);
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening presure.txt");
  }

delay(1000);
  }



// Thanks to Mike Grusin for letting me borrow the functions below from 
// the BMP180 example code. 

 double sealevel(double P, double A)
// Given a pressure P (mbar) taken at a specific altitude (meters),
// return the equivalent pressure (mbar) at sea level.
// This produces pressure readings that can be used for weather measurements.
{
	return(P/pow(1-(A/44330.0),5.255));
}


double altitude(double P, double P0)
// Given a pressure measurement P (mbar) and the pressure at a baseline P0 (mbar),
// return altitude (meters) above baseline.
{
	return(44330.0*(1-pow(P/P0,1/5.255)));
}

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
  while(1);
}

void Count_Time()
{
    static int ledState = LOW;
    count++;
    if (ledState == LOW) {
    ledState = HIGH;    
  } else {
    ledState = LOW;
  }
  digitalWrite(led, ledState);
    
  if( count  >= FlightTime ); // millis() returns the number of milliseconds the program has be running.  
  { 
    //Executes cutdown sequence.
    cutdown();
  }
  
}
