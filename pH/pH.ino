const int pHSensorPin = A0;
const float voltageAtpH7 = 3.295;  // Adjusted voltage at pH 7
const float slope = -0.18;        // Adjust slope based on your calibration data

void setup() {
  Serial.begin(9600);
  Serial.println("pH Sensor Test");
  delay(1000); // Wait for sensor stabilization
}

void loop() {
  int sensorValue = analogRead(pHSensorPin);   // Read analog value
  float voltage = sensorValue * (5.0 / 1023.0); // Convert to voltage

  // Calculate pH value based on voltage (linear approximation)
  float pH = 7.0 + ((voltage - voltageAtpH7) / slope);

  // Print the voltage and pH value
  Serial.print("Voltage: ");
  Serial.print(voltage, 2);
  Serial.print(" V, pH: ");
  Serial.println(pH, 2);

  delay(1000); // Delay between readings
}