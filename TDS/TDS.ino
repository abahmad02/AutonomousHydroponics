#include <SoftwareSerial.h>

// Define the pins for EC and pH sensors
const int ecPin = A1;         // Analog pin A1 for the EC sensor
const int pHSensorPin = A0;   // Analog pin A0 for the pH sensor
const int relayPin = 7;       // Pin connected to the relay module
const int motor1 = 2;
const int motor2 = 4;

// pH sensor calibration constants
const float voltageAtpH7 = 3.23;  // Adjusted voltage at pH 7
const float slope = -0.18;         // Adjust slope based on your calibration data

// SoftwareSerial pins for ESP communication
const int txPin = 5;  // TX pin connected to NodeMCU RX
const int rxPin = 6;  // RX pin connected to NodeMCU TX

SoftwareSerial espSerial(txPin, rxPin);  // Initialize SoftwareSerial

// EC calibration constants
const float referenceVoltage = 5.0;  // Microcontroller reference voltage (e.g., 5V)
const float ecStandard = 0.85;       // Calibration solution EC in mS/cm (194 μS/cm = 0.194 mS/cm)
float probeConstant = 1.0;           // Probe constant (K)

// EC to µS/cm conversion factor (from mS/cm to µS/cm)
const float ecToMicroSConversionFactor = 1000.0;  // 1 mS/cm = 1000 µS/cm

void setup() {
  Serial.begin(9600);       // Serial communication for debugging
  espSerial.begin(9600);    // Start SoftwareSerial communication
  
  pinMode(ECMotor, OUTPUT);
  pinMode(phMotorIncrease, OUTPUT);
  pinMode(pHMotorDecrease, OUTPUT);

  Serial.println("Calibrating EC sensor...");
  
  // EC sensor calibration to determine the probe constant
  int ecValue = analogRead(ecPin);  // Read raw ADC value from EC sensor
  float ecVoltage = ecValue * (referenceVoltage / 1023.0);  // Convert to voltage
  probeConstant = ecStandard / ecVoltage;  // Calculate the probe constant (K)
  
  Serial.print("Calibration completed. Probe constant (K): ");
  Serial.println(probeConstant, 6);  // Display K with high precision

  Serial.println("EC and pH Sensor Test");
  digitalWrite(ECMotor, HIGH);
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
  Serial.print(finalEc);  // Output EC in µS/cm
  Serial.println(" µS/cm");

  Serial.print("pH Value: ");
  Serial.println(pH, 2);  // Output pH value

  // Send EC and pH values to NodeMCU
  espSerial.print("EC Value: ");
  espSerial.print(finalEc);  // Send EC in µS/cm
  espSerial.println(" µS/cm");

  espSerial.print("pH Value: ");
  espSerial.println(pH, 2);

  // Relay control logic based on EC value
  if (ec < 900) {
    digitalWrite(relayPin, LOW);  // Turn relay off
    digitalWrite(motor1, LOW);
    digitalWrite(motor2, LOW);
  } else {
    digitalWrite(relayPin, HIGH);
    digitalWrite(motor1, HIGH);
    digitalWrite(motor2, HIGH);
      // Turn relay on
  }

  delay(1000);  // Wait for a second before the next reading
}
