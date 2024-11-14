#include <SoftwareSerial.h>

// Define the pin where the EC sensor is connected
const int ecPin = A1;  // Analog pin A1 for the sensor
const int relayPin = 7;  // Pin connected to the relay module

// Define pins for SoftwareSerial
const int txPin = 5;  // TX pin connected to NodeMCU RX
const int rxPin = 6;  // RX pin connected to NodeMCU TX

SoftwareSerial espSerial(txPin, rxPin);  // Initialize SoftwareSerial on pins 5 and 6

void setup() {
  Serial.begin(9600);       // Start Serial communication for debugging
  espSerial.begin(9600);    // Start SoftwareSerial at the same baud rate
  
  pinMode(relayPin, OUTPUT);  // Set the relay pin as an output
}

void loop() {
  int ecValue = analogRead(ecPin);  // Read the analog value from the sensor

  // Convert the raw reading to a meaningful value (adjust based on your specific EC sensor)
  float voltage = ecValue * (5.0 / 1023.0);  // Convert the reading to voltage
  float ec = voltage * 1000;  // Example conversion to mS/cm (adjust as needed)
  
  Serial.print("EC Value: ");
  Serial.print(ec);  // Print the EC value to the Serial Monitor
  Serial.println(" mS/cm");
  
  // Send the EC reading to the NodeMCU
  espSerial.print("EC Value: ");
  espSerial.print(ec);  // Send EC value over SoftwareSerial to NodeMCU
  espSerial.println(" mS/cm");
  
  // Relay control logic
  if (ec < 900) {
    digitalWrite(relayPin, LOW);  // Turn relay off
  } else {
    digitalWrite(relayPin, HIGH);  // Turn relay on
  }
  
  delay(1000);  // Wait for a second before the next reading
}
