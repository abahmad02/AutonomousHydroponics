#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi Access Point credentials
const char* ssid = "ESP8266_AP";
const char* password = "12345678";

// Create an instance of the web server on port 80
ESP8266WebServer server(80);

String ecReading = "0.0";  // Default EC reading
String pHReading = "7.0"; // Default pH reading

void setup() {
  Serial.begin(9600);   // Begin serial communication with Arduino

  // Set up ESP8266 as an access point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Serve the main HTML page
  server.on("/", []() {
    server.send(200, "text/html", R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <title>Sensor Data Chart</title>
        <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
        <script>
          let ecChart, pHChart;

          function createCharts() {
            const ctx1 = document.getElementById('ecChart').getContext('2d');
            const ctx2 = document.getElementById('pHChart').getContext('2d');
            
            ecChart = new Chart(ctx1, {
              type: 'line',
              data: {
                labels: [],  // Timestamps
                datasets: [{
                  label: 'EC (mS/cm)',
                  data: [],
                  borderColor: 'blue',
                  backgroundColor: 'rgba(0, 0, 255, 0.1)',
                  pointRadius: 4,
                  fill: true
                }]
              },
              options: { responsive: true, scales: { x: { display: true }, y: { beginAtZero: true } } }
            });

            pHChart = new Chart(ctx2, {
              type: 'line',
              data: {
                labels: [],  // Timestamps
                datasets: [{
                  label: 'pH',
                  data: [],
                  borderColor: 'green',
                  backgroundColor: 'rgba(0, 255, 0, 0.1)',
                  pointRadius: 4,
                  fill: true
                }]
              },
              options: { responsive: true, scales: { x: { display: true }, y: { beginAtZero: true } } }
            });
          }

          async function fetchData() {
            const response = await fetch('/data');
            const data = await response.json();
            const timestamp = new Date().toLocaleTimeString();

            // Update EC Chart
            ecChart.data.labels.push(timestamp);
            ecChart.data.datasets[0].data.push(data.ec);
            if (ecChart.data.labels.length > 20) {
              ecChart.data.labels.shift(); // Keep the latest 20 points
              ecChart.data.datasets[0].data.shift();
            }
            ecChart.update();

            // Update pH Chart
            pHChart.data.labels.push(timestamp);
            pHChart.data.datasets[0].data.push(data.pH);
            if (pHChart.data.labels.length > 20) {
              pHChart.data.labels.shift();
              pHChart.data.datasets[0].data.shift();
            }
            pHChart.update();
          }

          // Fetch data every 2 seconds
          setInterval(fetchData, 2000);

          window.onload = createCharts;
        </script>
      </head>
      <body>
        <h1>Sensor Data Chart</h1>
        <canvas id="ecChart" width="400" height="200"></canvas>
        <canvas id="pHChart" width="400" height="200"></canvas>
      </body>
      </html>
    )rawliteral");
  });

  // Route to provide sensor data as JSON
  server.on("/data", []() {
    String jsonResponse = "{ \"ec\": \"" + ecReading + "\", \"pH\": \"" + pHReading + "\" }";
    server.send(200, "application/json", jsonResponse);  // Send EC and pH readings as JSON
  });

  // Start the server
  server.begin();
  Serial.println("Web server started!");
}

void loop() {
  // Check for data from the Arduino
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');  // Read the data
    input.trim();  // Remove whitespace or newline

    // Parse EC or pH values
    if (input.startsWith("EC Value:")) {
      ecReading = input.substring(10);  // Extract EC value
      ecReading.trim();
      Serial.println("Updated EC value: " + ecReading);
    } else if (input.startsWith("pH Value:")) {
      pHReading = input.substring(10);  // Extract pH value
      pHReading.trim();
      Serial.println("Updated pH value: " + pHReading);
    }
  }

  server.handleClient();  // Handle client requests
}
