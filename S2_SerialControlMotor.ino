#include <Servo.h>

Servo motor[8];      // Array to hold the motor objects
Servo gripper;       // Add 1 servo for gripper
Servo rotategripper; // Add 1 servo for rotategripper
Servo rotategripper_v; // add 1 servo for rotategripper_v
int rotateGripperPin = 11;
int rotateGripperPin_v = 12;
String readString;      // String to hold incoming command from serial port
#define indicator 13    // Indicator LED pin
#define gripperOpen 150 // gripper open value
#define gripperClose 30 // gripper close value
#define gripperStop 93  // gripper stop value
int rotateangle = 90;   // rotategripper initial angle
int direction;
int rotateangle_v = 90;
int direction_y;
unsigned long previousMillis = 0;

void setup()
{
  Serial.begin(9600);         // Start serial communication at 115200 baud rate
  Serial2.begin(9600);        // Communication with the Arduino on shore
  pinMode(indicator, OUTPUT);   // Set the indicator LED pin as output
  digitalWrite(indicator, LOW); // Turn off the indicator LED
  gripper.attach(10);
  rotategripper.attach(rotateGripperPin);
  rotategripper_v.attach(rotateGripperPin_v);
  gripper.write(gripperStop);
  rotategripper.write(rotateangle);
  rotategripper_v.write(rotateangle_v);  
}

void loop()
{
  if (Serial2.available())
  {
    char c = Serial2.read(); // Get one byte from serial buffer
    if (c == '\r')
    {
      processCommand(readString); // Process the command
      readString = "";            // Clear the string for new input
    }
    else if (c == '\n')
    {
      // Do nothing
    }
    else
    {
      readString += c; // Build the string
    }
  }
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 16)
  {
    previousMillis = currentMillis;
    if (direction == 1 || direction == -1)
      rotateGripper();
    if (direction_y == 1 || direction_y == -1)
      rotateGripper_v();
  }
}

void processCommand(String command)
{
  char functionType = command.charAt(0); // Get the function type (M or F)
  command = command.substring(2);        // Remove the function type and comma from the command
  int commaIndex = command.indexOf(','); // Find the index of the first comma
  // Serial.println("Function type: " + functionType);
  if (functionType == 'M')
  {
    // Motor control function
    Serial.println("Motor speed: " + command);
    int i = 0;
    while (commaIndex != -1 && i < 8)
    {
      int value = command.substring(0, commaIndex).toInt(); // Get the value before the comma
      motor[i].write(value);                                // Control the motor with the value
      command = command.substring(commaIndex + 1);          // Remove the processed part from the command
      commaIndex = command.indexOf(',');                    // Find the next comma
      i++;
    }
    // Process the last value if there are less than 8 motors
    if (i < 8)
    {
      int lastValue = command.toInt();
      motor[i].write(lastValue); // Control the motor with the last value
    }
  }
  else if (functionType == 'F')
  {
    // Toggle function
    int functionNumber = command.substring(0, commaIndex).toInt(); // Get the function number
    int functionState = command.substring(commaIndex + 1).toInt(); // Get the function state
    // Call your toggle function here with the function number and state
    toggleFunction(functionNumber, functionState);
  }
  else if (functionType == 'S')
  {
    // Stop all motors
    for (int i = 0; i < 8; i++)
    {
      motor[i].write(90); // Stop the motor
    }
    Serial.println("All motors stopped");
  }
  else if (functionType == 'H') // trigger when hat button is pressed
  {
  }
  else if (functionType == 'P')
  {
    // Ping function
    Serial2.println("P");
  }
  else
  {
    Serial.println("Invalid function type");
  }
}

void GetpHValue() // function for getting temperature
{
  float measure = analogRead(A4);  //Read pin A0
  double voltage = measure*5/1024; //Analog-to-Digital Conversion
 
  // PH_step (Voltage/pH Unit) = (Voltage@PH7-Voltage@PH4)/(PH7 - PH4)
  float pH = 7+((2.5 - voltage)/0.1841); // PH_probe = PH7-((Voltage@PH7-Voltage@probe)/PH_step)
  Serial2.print("P="); //Print word pH in Serial Monitor
  Serial2.println(pH); //Print pH value in Serial Monitor  
}

void rotateGripper()
{
  if (direction == 1)
  {
    rotateangle = rotateangle + 2;
    if (rotateangle >= 180)
      rotateangle = 180;
    rotategripper.write(rotateangle);
  }
  else if (direction == -1)
  {
    rotateangle = rotateangle - 2;
    if (rotateangle <= 0)
      rotateangle = 0;
    rotategripper.write(rotateangle);
  }
  else if (direction == 0)
  {
  }
}

void rotateGripper_v()
{
  if (direction_y == 1)
  {
    rotateangle_v = rotateangle_v + 1;
    if (rotateangle_v >= 160)
      rotateangle_v = 160;
    rotategripper_v.write(rotateangle_v);
  }
  else if (direction_y == -1)
  {
    rotateangle_v= rotateangle_v - 1;
    if (rotateangle_v <= 70)
      rotateangle_v = 70;
    rotategripper_v.write(rotateangle_v);
  }
  else if (direction_y == 0)
  {
  }
}

// Function to toggle a specific function based on the function number and state
void toggleFunction(int functionNumber, int functionState)
{
  // Implement your toggle function here
  Serial.println("Function " + (String)functionNumber + " toggled to state " + (String)functionState);
  switch (functionNumber)
  {
  case 1:
    // Button X, Blue
    if (functionState == 1)
    {
      direction_y = 1;
      rotateGripper_v();
    }
    else
    {
      direction_y = 0;
      rotateGripper_v();
    }
    break;
  case 2:
    // Button A, Green
    if (functionState == 1)
    {
      GetpHValue();
    }
    break;
  case 3:
    // Button B, Red
    break;
  case 4:
    // Button Y, Yellow
    if (functionState == 1)
    {
      direction_y = -1;
      rotateGripper_v();
    }
    else
    {
      direction_y = 0;
      rotateGripper_v();
    }
    break;
  case 5:
    // Button L1, Left bumper
    if (functionState == 1)
    {
      direction = 1;
      rotateGripper();
    }
    else
    {
      direction = 0;
      rotateGripper();
    }
    break;
  case 6:
    // Button R1, Right bumper
    if (functionState == 1)
    {
      gripper.write(gripperStop + 60); // open the gripper
    }
    else
    {
      gripper.write(gripperStop); // stop the gripper
    }
    break;
  case 7:
    // Button L2, Left trigger
    if (functionState == 1)
    {
      direction = -1;
      rotateGripper();
    }
    else
    {
      direction = 0;
      rotateGripper();
    }
    break;
  case 8:
    // Button R2, Right trigger
     if (functionState == 1)
     {
      gripper.write(gripperStop - 60); // close the gripper
     }
     else
     {
      gripper.write(gripperStop); // stop the gripper
     }
    break;
  case 9:
    // Button Back, detach all motors
    for (int i = 0; i < 8; i++)
    {
      motor[i].detach();
    }
    digitalWrite(indicator, LOW); // Turn off the indicator LED
    Serial.println("Disarmed!");
    break;
  case 10:
    // Button Start, attach all motors and set to 90
    for (int i = 0; i < 8; i++)
    {
      motor[i].attach(i + 2);
      motor[i].write(90);
    }
    delay(1000);                   // wait for 1 second
    digitalWrite(indicator, HIGH); // Turn on the indicator LED
    Serial.println("Armed and ready!");
    break;
  case 11:
    // Button L3, Left stick button
    break;
  case 12:
    // Button R3, Right stick button
    break;
  default:
    // Invalid function number
    Serial.println("Invalid function number");
    break;
  }
}
