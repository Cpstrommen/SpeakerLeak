//libraries
#include <Wire.h>

//pressure sensor definitions
#define outputRange   13107 // This is OUTPUTmax - OUTPUTmin
#define pressureRange 60   // This is PRESSUREmax - PRESSUREmin
#define OUTPUTmin     1638  // OUTPUTmin (counts)
#define PRESSUREmin   30    //MINIMUM PRESSURE IS -30 

//changing variables
const float cutoff = 0.1; //amount of pressure to be the cutoff between pass/fail
const float lowPressure = 0.1; //amount of pressure to be determined as low pressure (number in psi)

//pin variables
const int buttonPin = 5;
const int actuatorPin = 2;
const int pressurePin = 9;
const int ledPin = 3;

//pressure determination variables
float initialPressure, endPressure, change, pressureIn, numerator, numerator1, tempCalc1, tempCalc2, Temp;

//pressure sensor variables
int address = 40;  //decimal address of sensor 1
int X0, X1, X2;
int outPut = 0;

//delay variables
int count = 0;

void setup() {

  //serial initializing
  Serial.begin(9600);

  //setting pin modes
  pinMode(buttonPin, INPUT);
  pinMode(actuatorPin, OUTPUT);
  pinMode(pressurePin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  //wire library
  Wire.begin();

  //start message
  Serial.println("Test Ready...Press Button to Start");
}

void loop() {
  
  //resetting the initial and end pressure after every run
  initialPressure = 0;
  endPressure = 0;

    //waiting for a button press
    if (digitalRead(buttonPin) == HIGH){

      //serial
      Serial.println("Initiating Test..."); 

      //turning on the actuator and off the pressure relay
      digitalWrite(actuatorPin, HIGH);
      digitalWrite(pressurePin, LOW);
      delay(1000);
      
      //turning both the actuator and the pressure on
      digitalWrite(actuatorPin, HIGH);
      digitalWrite(pressurePin, HIGH);
      delay(1000);

//------GETTING THE PRESSURE READING START -------------------------------------------------------------------

        //waking up wire line
        Wire.requestFrom(address, 3);

        //reading wire values
        if (Wire.available() <= 3) {
          X0 = Wire.read(); // Reads the data from the register
          X1 = Wire.read();
          X2 = Wire.read();
        }
        
        outPut = ((X0 << 8) + X1); //Concatenate first 2 bytes

        //calculating the pressure
        numerator = (outPut - OUTPUTmin);
        numerator1 = numerator * 10;
        pressureIn = (numerator1 / outputRange) - 5;
        tempCalc1 = (X2 << 3);
        tempCalc2 = tempCalc1 / (float)2047;
        Temp = (tempCalc2 * 200) - 50;
    
        initialPressure = pressureIn;
        
//------END OF PRESSURE---------------------------------------------------------------------------------------------

        //checking to see if ther is pressure in the system
        if (initialPressure < lowPressure) {
          Serial.println("Error: No pressure in the system");
        }

        //if so, continue, if not, stop
        if (initialPressure > lowPressure) {

          //holding both relays open
          digitalWrite(actuatorPin, HIGH);
          digitalWrite(pressurePin, HIGH);
          delay(1000);
  
          //closing the pressure relay
          digitalWrite(actuatorPin, HIGH);
          digitalWrite(pressurePin, LOW);
          Serial.print("Running Test...");

          while(count < 20){
            delay(1000);
            Serial.print(count + 1);
            Serial.print(".");
            count++;
          }
          Serial.println();

//--------GETTING A PRESSURE READING START-------------------------------------------------------------

          //waking up wire line
          Wire.requestFrom(address, 3);

          //reading all data
          if (Wire.available() <= 3) {
            X0 = Wire.read(); // Reads the data from the register
            X1 = Wire.read();
            X2 = Wire.read();
          }
        
          outPut = ((X0 << 8) + X1); //Concatenate first 2 bytes
          numerator = (outPut - OUTPUTmin);
          numerator1 = numerator * 10;

          //calculating the pressure and temp
          pressureIn = (numerator1 / outputRange) - 5;
          tempCalc1 = (X2 << 3);
          tempCalc2 = tempCalc1 / 2047;
          Temp = (tempCalc2 * 200) - 50;
        
          endPressure = pressureIn;
          
//--------END OF PRESSURE---------------------------------------------------------------------

          //closing the actuator relay and opening up the pressure relay
          digitalWrite(actuatorPin, LOW);
          digitalWrite(pressurePin, HIGH);

          //calculating the change in pressure
          change = initialPressure - endPressure;
          Serial.print("Change in Pressure: ");
          Serial.println(change);

          //determining pass or fail
          if (change > cutoff) {
            digitalWrite(ledPin, LOW);
            Serial.println("FAIL");
          }
          if (change < cutoff) {
            digitalWrite(ledPin, HIGH);
            Serial.println("PASS");
          }

          //resetting the relay pins
          digitalWrite(actuatorPin, LOW);
          digitalWrite(pressurePin, LOW);

          //serial ready to start again
          Serial.println("Test Ready...Press Button to Start");
        } 
    }
}
