#include <SoftwareSerial.h>

unsigned long motorStartTime = 0;  // Motorun çalışmaya başladığı zamanı kaydetmek için
bool motorRunning = false;        // Motorun çalışıp çalışmadığını takip etmek için

// Define the pins for EC and pH sensors
const int ecPin = A1;         // Analog pin A1 for the EC sensor
const int pHSensorPin = A0;   // Analog pin A0 for the pH sensor
const int relayPin = 7;       // Pin connected to the relay module
const int motor1 = 2;
const int motor2 = 4;

// pH sensor calibration constants
const float voltageAtpH7 = 3.23;  // Adjusted voltage at pH 7
const float slope = -0.18;         // Adjust slope based on your calibration data

// Define pins for SoftwareSerial
const int txPin = 5;  // TX pin connected to NodeMCU RX
const int rxPin = 6;  // RX pin connected to NodeMCU TX

SoftwareSerial espSerial(txPin, rxPin);  // Initialize SoftwareSerial on pins 5 and 6

void setup() {
  Serial.begin(9600);       // Start Serial communication for debugging
  espSerial.begin(9600);    // Start SoftwareSerial at the same baud rate
  
  pinMode(relayPin, OUTPUT);
  pinMode(motor1, OUTPUT);  // Set the relay pin as an output
  pinMode(motor2, OUTPUT);

  Serial.println("EC and pH Sensor Test");
  delay(1000);  // Wait for stabilization
}

void loop() {
  // Read EC sensor value
  int ecValue = analogRead(ecPin);  // Read the analog value from the EC sensor
  float ecVoltage = ecValue * (5.0 / 1023.0);  // Convert the reading to voltage
  float ec = ecVoltage * 1000;  // Example conversion to mS/cm (adjust as needed)
  
  // Read pH sensor value
  int pHSensorValue = analogRead(pHSensorPin);  // Read the analog value from the pH sensor
  float pHVoltage = pHSensorValue * (5.0 / 1023.0);  // Convert to voltage
  float pH = 7.0 + ((pHVoltage - voltageAtpH7) / slope);  // Calculate pH value

  // Print EC and pH values to the Serial Monitor
  Serial.print("EC Value: ");
  Serial.print(ec);
  Serial.println(" mS/cm");

  Serial.print("pH Value: ");
  Serial.println(pH, 2);

  // Send EC and pH values to the NodeMCU
  espSerial.print("EC Value: ");
  espSerial.print(ec);
  espSerial.println(" mS/cm");

  espSerial.print("pH Value: ");
  espSerial.println(pH, 2);

/*
  // Relay control logic based on EC value
if (ec < 900) {
    digitalWrite(relayPin, LOW);  // Turn relay off

  } else {
    digitalWrite(relayPin, HIGH);
      // Turn relay on
      
  }
  
  if (pH < 4.0) {
    digitalWrite(motor2, HIGH);
  }
  else if (pH > 5.0){
    digitalWrite(motor2, LOW);
  } 



  if (pH > 5.5 && !motorRunning) {  // pH **'dan büyük ve motor çalışmıyorsa
    digitalWrite(motor1, HIGH);     // Motoru çalıştır
    motorStartTime = millis();      // Başlangıç zamanını kaydet
    motorRunning = true;            // Motor çalışıyor durumuna geç
  }

  if (motorRunning && millis() - motorStartTime >= 15000) { // 15 saniye geçtiyse
    digitalWrite(motor1, LOW);       // Motoru durdur
    motorRunning = false;            // Motor durdu
  }


*/    
  delay(1000);  // Wait for a second before the next reading
}
