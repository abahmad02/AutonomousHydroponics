#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi Access Point credentials
const char* ssid = "ESP8266_AP";
const char* password = "12345678";

// Create an instance of the web server on port 80
ESP8266WebServer server(80);

String ecReading = "Waiting for data...";  // Initial EC reading

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
          <title>EC Sensor Data</title>
          <script>
            // Function to request EC data from the server and update the page
            function fetchECData() {
              fetch("/ec")
                .then(response => response.text())
                .then(data => {
                  document.getElementById("ecValue").innerText = data + " mS/cm";
                })
                .catch(error => console.error("Error fetching EC data:", error));
            }
            // Fetch EC data every 2 seconds
            setInterval(fetchECData, 2000);
          </script>
        </head>
        <body>
          <h1>EC Sensor Data</h1>
          <p>EC Value: <span id="ecValue">Waiting for data...</span></p>
        </body>
      </html>
    )rawliteral");
  });

  // Route to handle AJAX request for EC data
  server.on("/ec", []() {
    server.send(200, "text/plain", ecReading);  // Send the EC reading only
  });

  // Start the server
  server.begin();
  Serial.println("Web server started!");
}

void loop() {
  // Check if there's data available from the Arduino
  if (Serial.available()) {
    ecReading = Serial.readStringUntil('\n');  // Read the data from the Arduino
    Serial.println("Received from Arduino: " + ecReading);
  }

  server.handleClient();  // Handle web server client
}
