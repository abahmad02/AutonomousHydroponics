#include <SoftwareSerial.h>

// Global variables
unsigned long motorStartTime = 0;  // Motor start time
bool motorRunning = false;         // Motor status
unsigned long lastPHCheckTime = 0; // Last time pH was checked
bool pHCheckEnabled = true;        // Allow pH checking

// EC and pH sensor pins
const int ecPin = A1;             // EC sensor on A1
const int pHSensorPin = A0;       // pH sensor on A0
const int ECMotor = 7;            // Relay control pin
const int phMotorIncrease = 2;    // Motor for increasing pH
const int pHMotorDecrease = 4;    // Motor for decreasing pH

// pH calibration constants
const float voltageAtpH7 = 3.35;  // Voltage at pH 7
const float slope = -0.18;        // Slope for pH calculation

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
  delay(1000);  // Wait for stabilization
}

void loop() {
  // EC sensor reading and conversion to µS/cm
  int ecValue = analogRead(ecPin);  // EC sensor analog read
  float ecVoltage = ecValue * (5.0 / 1023.0);  // Convert to voltage
  float ec = ecVoltage * probeConstant;  // Calculate EC in mS/cm using probe constant
  float microS = ec * ecToMicroSConversionFactor;  // Convert to µS/cm

  // pH sensor reading and calculation
  int pHSensorValue = analogRead(pHSensorPin);  // Read pH sensor value
  float pHVoltage = pHSensorValue * (5.0 / 1023.0);  // Convert to voltage
  float pH = 7.0 + ((pHVoltage - voltageAtpH7) / slope);  // Calculate pH value

  // Adjusted EC calculation for output
  int finalEc = ecValue * 3;  // Placeholder adjustment
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

  // Trigger ECMotor if finalEc <= 150
  if (finalEc <= 150) {
    Serial.println("EC Value below threshold. Activating ECMotor...");
    digitalWrite(ECMotor, HIGH);  // Turn on the motor
    delay(2000);                  // Keep the motor on for 2 seconds
    digitalWrite(ECMotor, LOW);   // Turn off the motor
    Serial.println("ECMotor deactivated.");
  }

  // Check pH if pH checks are enabled
  if (pHCheckEnabled && (pH < 5.5 || pH > 6.5)) {
    unsigned long startTime = millis();
    bool withinRange = false;

    Serial.println("pH out of range. Monitoring for 10 seconds...");
    while (millis() - startTime < 10000) {
      pHSensorValue = analogRead(pHSensorPin);
      pHVoltage = pHSensorValue * (5.0 / 1023.0);
      pH = 7.0 + ((pHVoltage - voltageAtpH7) / slope);

      Serial.print("Current pH Value: ");
      Serial.println(pH, 2);

      if (pH >= 5.5 && pH <= 6.5) {
        Serial.println("pH back within range.");
        withinRange = true;
        break;
      }

      delay(1000);  // Check every second
    }

    if (!withinRange) {
      if (pH > 6.5) {
        Serial.println("pH too high. Activating pHMotorDecrease...");
        digitalWrite(pHMotorDecrease, HIGH);
        delay(5000);
        digitalWrite(pHMotorDecrease, LOW);
      } else if (pH < 5.5) {
        Serial.println("pH too low. Activating phMotorIncrease...");
        digitalWrite(phMotorIncrease, HIGH);
        delay(5000);
        digitalWrite(phMotorIncrease, LOW);
      }

      Serial.println("Waiting 5 minutes to allow pH solutions to mix...");
      pHCheckEnabled = false;  // Disable pH checking
      lastPHCheckTime = millis();
    }
  }

  // Re-enable pH checking after 5 minutes
  if (!pHCheckEnabled && millis() - lastPHCheckTime >= 300000) {
    Serial.println("pH checking re-enabled.");
    pHCheckEnabled = true;
  }

  delay(1000);  // Wait for a second before the next reading
}
