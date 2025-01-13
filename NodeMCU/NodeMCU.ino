#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

// WiFi Access Point credentials
const char* ssid = "ESP8266_AP";       // SSID for ESP8266's AP
const char* password = "12345678";     // Password for ESP8266's AP

// Flask server details (Update with your PC's IP address)
const char* serverIP = "192.168.4.2";  // PC's IP on ESP8266 network
const int serverPort = 5000;           // Flask server port

// Variables for sensor data
String ecReading = "0";   // EC value placeholder
String pHReading = "7";   // pH value placeholder
String incomingData = ""; // String to hold incoming data

void setup() {
  Serial.begin(9600);  // Initialize serial communication

  // Set up ESP8266 as an access point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("ESP8266 AP IP address: ");
  Serial.println(IP);

  delay(1000);  // Give time for the ESP to initialize
}

void loop() {
  // Read incoming data from Arduino
  while (Serial.available()) {
    char incomingChar = Serial.read();  // Read a single character

    if (incomingChar == '\n') {
      // If a newline character is encountered, process the data
      Serial.print("Raw Input: ");
      Serial.println(incomingData);  // Print the raw input for debugging

      // Parse EC and pH values
      if (incomingData.startsWith("EC Value:")) {
        ecReading = incomingData.substring(10);  // Extract EC value
        ecReading.trim();  // Remove any extra spaces
        ecReading = extractNumericValue(ecReading);  // Ensure it's a numeric value
        Serial.print("EC Reading: ");
        Serial.println(ecReading);  // Print to Serial Monitor for verification
      } else if (incomingData.startsWith("pH Value:")) {
        pHReading = incomingData.substring(10);  // Extract pH value
        pHReading.trim();  // Remove any extra spaces
        Serial.print("pH Reading: ");
        Serial.println(pHReading);  // Print to Serial Monitor for verification
      }

      // Clear the incoming data buffer for the next message
      incomingData = "";
    } else {
      // Accumulate characters until a newline is encountered
      incomingData += incomingChar;
    }
  }

  // Send data to Flask server
  sendDataToFlaskServer();

  delay(2000);  // Add some delay between readings
}

// Function to extract only numeric values from the input string (e.g., removing " µS/cm")
String extractNumericValue(String str) {
  String result = "";
  for (int i = 0; i < str.length(); i++) {
    if (isDigit(str[i]) || str[i] == '.') {
      result += str[i];  // Add numeric characters or period to the result
    }
  }
  // Return the result or default to "0" if no valid number was found
  if (result == "") {
    result = "0";
  }
  return result;
}

// Function to send data to Flask server
void sendDataToFlaskServer() {
  WiFiClient client;

  if (client.connect(serverIP, serverPort)) {
    // Prepare JSON data to send
    String jsonData = "{\"ec\":\"" + ecReading + "\", \"pH\":\"" + pHReading + "\"}";

    // Debugging: Print out the JSON data being sent to Flask
    Serial.print("Sending data: ");
    Serial.println(jsonData);

    // Send HTTP POST request to Flask server
    client.println("POST /update HTTP/1.1");
    client.println("Host: " + String(serverIP));
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonData.length());
    client.println();  // End of headers
    client.println(jsonData);  // Send the actual JSON data

    // Read server response
    while (client.connected()) {
      while (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);  // Print server response for debugging
      }
    }

    // Close the connection to the server
    client.stop();
  } else {
    Serial.println("Failed to connect to Flask server.");
  }
}