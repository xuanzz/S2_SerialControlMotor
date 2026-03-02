#include <Servo.h>
#include <SoftwareSerial.h>

Servo motor[13];      // Array to hold the motor objects, motor[0] to motor[5] for motors, motor[6] for gripper, motor[7] for rotategripper, motor[8] for rotategripper_v, motot[9] for check sum of motors
Servo gripper;       // Add 1 servo for gripper
Servo rotategripper; // Add 1 servo for rotategripper
Servo rotategripper_v; // add 1 servo for rotategripper_v
Servo widthScanner;  //Servo for scanning width
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
int motorStop[11] = {91,91,92,92,92,92,0,0,0,0,0}; //medium value
int lastMotorValue[11] = {91,91,92,92,92,92,0,0,0,0,0}; // last motor value
SoftwareSerial a02Serial1(,); // RX, TX
SoftwareSerial a02Serial2(,); // RX, TX
unsigned char data[4]={};
float distance;
unsigned char data[4];
//int distance = 0;
//int leftAngle = 0;
//int rightAngle = 0;
//bool objectDetected = false;
uint16_t horizontalDistance = 0;
uint16_t height = 0;
int width = 0;
const uint8_t HEADER_BYTE_1 = 0xFF;
const uint8_t HEADER_BYTE_2 = 0xFF;
const uint16_t READ_TIMEOUT_MS = 200;
const int ERROR_DISTANCE = -1;  // Error return value



void setup()
{
  Serial.begin(115200);         // Start serial communication at 115200 baud rate
  Serial2.begin(9600);        // Communication with the Arduino on shore
  a02Serial1.begin(9600);       // Communication with the A02 sonar sensor
  a02Serial2.begin(9600);       // Communication with the A02 sonar sensor
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

uint8_t readN1(uint8_t *buf, size_t len) {
  size_t offset = 0, left = len;
  uint8_t *buffer = buf;
  long curr = millis();
  while (left) {
    if (a02Serial1.available()) {
      buffer[offset] = a02Serial1.read();
      offset++;
      left--;
    }
    if (millis() - curr > READ_TIMEOUT_MS) {
      break;
    }
  }
  return offset;
}

uint8_t readN2(uint8_t *buf, size_t len) {
  size_t offset = 0, left = len;
  uint8_t *buffer = buf;
  long curr = millis();
  while (left) {
    if (a02Serial2.available()) {
      buffer[offset] = a02Serial2.read();
      offset++;
      left--;
    }
    if (millis() - curr > READ_TIMEOUT_MS) {
      break;
    }
  }
  return offset;
}


void processCommand(String command)
{
  char functionType = command.charAt(0); // Get the function type (M or F)
  command = command.substring(2);        // Remove the function type and comma from the command
  int commaIndex = command.indexOf(','); // Find the index of the first comma
  // Serial.println("Function type: " + functionType);
  //Serial.println("Command: " + command); // Print the command for debugging
  if (functionType == 'M')
  {
    int motorValue[11] = {0}; // Initialize motor values
    int i = 0;

    // Parse motor values from the command
    while (commaIndex != -1 && i < 11)
    {
      motorValue[i++] = command.substring(0, commaIndex).toInt();
      command = command.substring(commaIndex + 1);
      commaIndex = command.indexOf(',');
    }
    // Serial.print("Motor values: ");
    // for (int j = 0; j < i; j++)
    // {
    //   Serial.print("M-");
    //   Serial.print(j);
    //   Serial.print(": ");
    //   Serial.print(motorValue[j]);
    //   Serial.print(" ");
    // }
    //Serial.println();
    // Validate motorValue[10] is the sum of motorValue[0] to motorValue[9]
    if (i == 11 && motorValue[10] != motorValue[0] + motorValue[1] + motorValue[2] + motorValue[3] + motorValue[4] + motorValue[5] + motorValue[6] + motorValue[7] + motorValue[8] + motorValue[9])
    {
      Serial.println("Checksum error!"); // Print checksum error message
    }
    else
    { // Update motor values if changed
      for (int j = 0; j < 6; j++)
      {
        if (motorValue[j] != lastMotorValue[j])
        {
          motor[j].write(motorValue[j]);
          lastMotorValue[j] = motorValue[j];
        }
      }
      for (int j = 5; j < i; j++)
      {
        if (motorValue[j] != lastMotorValue[j])
        {
          int functionNumber = motorValue[j]/10; // Get the function number from the motor value
          int functionState = motorValue[j]%10; // Get the function state from the motor value
          toggleFunction(functionNumber, functionState);
          lastMotorValue[j] = motorValue[j];
        }
      }
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
  Serial2.print("P,"); //Print word pH in Serial Monitor
  Serial2.println(pH); //Print pH value in Serial Monitor  
}

void sonar()
{
  int readDistance(void) {
  uint8_t data[4] = { 0 };
  uint8_t receivedByte = 0;
  unsigned long startTime = millis();
  while (millis() - startTime < READ_TIMEOUT_MS) {                      // Check if timeout
    if (readN1(&receivedByte, 1) == 1 && receivedByte == HEADER_BYTE_1) {  // Find the header byte
      data[0] = receivedByte;
      if (readN1(&data[1], 3) == 3) {                     // Read the remaining 3 bytes
        uint8_t checksum = data[0] + data[1] + data[2];  // Checksum
        if (checksum == data[3]) {
          horizontalDistance = (data[1] << 8) | data[2];  // Calculate and return the distance
        }
      }
    }
    Serial.println("Error data");
  }
    Serial.println("Error Reading data timeout");
    return ERROR_DISTANCE;
  }

  int readHeight(void) {
  uint8_t data[4] = { 0 };
  uint8_t receivedByte = 0;
  unsigned long startTime = millis();
  while (millis() - startTime < READ_TIMEOUT_MS) {                      // Check if timeout
    if (readN2(&receivedByte, 1) == 1 && receivedByte == HEADER_BYTE_2) {  // Find the header byte
      data[0] = receivedByte;
      if (readN2(&data[1], 3) == 3) {                     // Read the remaining 3 bytes
        uint8_t checksum = data[0] + data[1] + data[2];  // Checksum
        if (checksum == data[3]) {
          height = (data[1] << 8) | data[2];  // Calculate and return the distance
        }
      }
    }
    Serial.println("Error data");
  }
    Serial.println("Error Reading data timeout");
    return ERROR_DISTANCE;
  }
  
  void measureWidth()
  {
    widthscanner.write(90);
    if (horizontalDistance > 0 && horizontalDistance < 800) {
      objectDetected = true;
      for (int angle = 90; angle < rightAngle; angle++) {
        widthscanner.write(angle);
        if (!objectDetected) {
          rightSAngle = angle-1;
          widthscanner.write(90);
        }
      }
      if (rightAngle > 90){
        for (int angle = 90; angle > leftAngle; angle--) {
          widthscanner.write(angle);
          if (!objectDetected) {
            leftAngle = angle-1;
            widthscanner.write(90);
            Serial.print("Width: ");
            width = horizontalDistance * tan(rightAngle-90) + horizontalDistance * tan(leftAngle);
            Serial.print(width);
            Serial.println(" mm");
          }
        }
      }
    }
  }
  void measureHeight() {
    for (int i = 0; i < 5; i++) {
      horizontalDistance = readDistance();
      if (!objectDetected) {
        if (horizontalDistance > 0 && horizontalDistance < 800) { // object detected threshold
          objectDetected = true;
          if (objectDetected == true) {
            int height = readHeight();
            Serial.print("Height: ");
            Serial.print(height);
            Serial.println(" mm");
            break;
          }
        }
      }    
    }    
  }  
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
      motor[i].write(motorStop[i]); // Set the motor to stop position
    }
    //delay(1000);                   // wait for 1 second
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
