#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi Access Point credentials
const char* ssid = "ESP8266_AP";
const char* password = "12345678";

// Create an instance of the web server on port 80
ESP8266WebServer server(80);

String ecReading = "Waiting for data...";  // Initial EC reading
String pHReading = "Waiting for data...";  // Initial pH reading

void setup() {
  Serial.begin(9600);   // Begin serial communication with Arduino

  // Set up ESP8266 as an access point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Route to serve the main HTML page
  server.on("/", []() {
    server.send(200, "text/html", R"rawliteral(
      <html>
        <head>
          <title>Sensor Data</title>
          <script>
            // Function to request sensor data and update the page
            function fetchData() {
              fetch("/data")
                .then(response => response.json())
                .then(data => {
                  document.getElementById("ecValue").innerText = data.ec + " mS/cm";
                  document.getElementById("pHValue").innerText = data.pH;
                })
                .catch(error => console.error("Error fetching sensor data:", error));
            }
            // Fetch data every 2 seconds
            setInterval(fetchData, 2000);
          </script>
        </head>
        <body>
          <h1>Sensor Data</h1>
          <p><strong>EC Value:</strong> <span id="ecValue">Waiting for data...</span></p>
          <p><strong>pH Value:</strong> <span id="pHValue">Waiting for data...</span></p>
        </body>
      </html>
    )rawliteral");
  });

  // Route to handle sensor data requests
  server.on("/data", []() {
    String jsonResponse = "{ \"ec\": \"" + ecReading + "\", \"pH\": \"" + pHReading + "\" }";
    server.send(200, "application/json", jsonResponse);  // Send EC and pH readings as JSON
  });

  // Start the server
  server.begin();
  Serial.println("Web server started!");
}

void loop() {
  // Check if there's data available from the Arduino
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');  // Read the data from the Arduino
    input.trim();  // Remove any extra whitespace or newline characters

    // Parse EC or pH values
    if (input.startsWith("EC Value:")) {
      ecReading = input.substring(10);  // Extract EC value
      ecReading.trim();  // Remove any extra whitespace
      Serial.println("Updated EC value: " + ecReading);
    } else if (input.startsWith("pH Value:")) {
      pHReading = input.substring(10);  // Extract pH value
      pHReading.trim();  // Remove any extra whitespace
      Serial.println("Updated pH value: " + pHReading);
    }

  }

  server.handleClient();  // Handle web server client
}
