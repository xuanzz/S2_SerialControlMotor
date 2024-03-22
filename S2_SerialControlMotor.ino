#include <Servo.h>

Servo motor[8]; // Array to hold the motor objects
String readString; // String to hold incoming command from serial port

void setup() {
  Serial.begin(115200); // Start serial communication at 115200 baud rate
  Serial2.begin(115200); // Communication with the Arduino on shore
  for (int i = 0; i < 8; i++) {
    motor[i].attach(i + 2); // Attach the motors to pins 2 through 9
  }
}

void loop() {
  if (Serial2.available()) {
    char c = Serial2.read(); // Get one byte from serial buffer
    if (c == '\n') {
      processCommand(readString); // Process the command
      readString = ""; // Clear the string for new input
    } else {
      readString += c; // Build the string
    }
  }
}

void processCommand(String command) {
  char functionType = command.charAt(0); // Get the function type (M or F)
  command = command.substring(2); // Remove the function type and comma from the command
  int commaIndex = command.indexOf(','); // Find the index of the first comma
  Serial.print("Function type: ");
  Serial.println(functionType);
  if (functionType == 'M') {
    // Motor control function
    Serial.println("Motor speed: " + command);
    int i = 0;
    while (commaIndex != -1 && i < 8) {
      int value = command.substring(0, commaIndex).toInt(); // Get the value before the comma
      motor[i].write(value); // Control the motor with the value
      command = command.substring(commaIndex + 1); // Remove the processed part from the command
      commaIndex = command.indexOf(','); // Find the next comma
      i++;
    }
    // Process the last value if there are less than 8 motors
    if (i < 8) {
      int lastValue = command.toInt();
      motor[i].write(lastValue); // Control the motor with the last value
    }
  } else if (functionType == 'F') {
    // Toggle function
    int functionNumber = command.substring(0, commaIndex).toInt(); // Get the function number
    int functionState = command.substring(commaIndex + 1).toInt(); // Get the function state
    // Call your toggle function here with the function number and state
    toggleFunction(functionNumber, functionState);
  } else if (functionType == 'S') {
    // Stop all motors
    for (int i = 0; i < 8; i++) {
      motor[i].write(90); // Stop the motor
    }
    Serial.println("All motors stopped");
  } else {
    Serial.println("Invalid function type");
  }
}

// Function to toggle a specific function based on the function number and state
void toggleFunction(int functionNumber, int functionState) {
  // Implement your toggle function here
  Serial.print("Function " + (String)functionNumber + " toggled to state " + (String)functionState);
}