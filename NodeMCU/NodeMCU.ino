#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>  // Include SPIFFS library

// WiFi Access Point credentials
const char* ssid = "ESP8266_AP";
const char* password = "12345678";

// Web server
ESP8266WebServer server(80);

// Circular buffer for data (store last 100 entries)
#define MAX_ENTRIES 100
String ecLog[MAX_ENTRIES];
String pHLog[MAX_ENTRIES];
String timeLog[MAX_ENTRIES];
int currentIndex = 0;

void setup() {
  Serial.begin(9600);  // Begin serial communication
  WiFi.softAP(ssid, password);  // Set up the ESP as an access point
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Route to serve the main HTML page
  server.on("/", []() {
    server.send(200, "text/html", R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <title>Sensor Data Chart</title>
        <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
        <script>
          let ecChart, pHChart;

          // Initialize the charts
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

          // Fetch data and update the charts
          async function fetchData() {
            const response = await fetch('/history');
            const data = await response.json();

            // Update EC chart
            ecChart.data.labels = data.map(entry => entry.time);
            ecChart.data.datasets[0].data = data.map(entry => entry.ec);
            ecChart.update();

            // Update pH chart
            pHChart.data.labels = data.map(entry => entry.time);
            pHChart.data.datasets[0].data = data.map(entry => entry.pH);
            pHChart.update();
          }

          // Refresh data every 5 seconds
          setInterval(fetchData, 5000);

          // Load charts after the page loads
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

  // Route to handle historical data
  server.on("/history", []() {
    String historyJson = "[";
    for (int i = 0; i < MAX_ENTRIES; i++) {
      if (ecLog[i].length() > 0 && pHLog[i].length() > 0) {
        if (i > 0) historyJson += ",";
        historyJson += "{";
        historyJson += "\"time\":\"" + timeLog[i] + "\",";
        historyJson += "\"ec\":" + ecLog[i] + ",";
        historyJson += "\"pH\":" + pHLog[i];
        historyJson += "}";
      }
    }
    historyJson += "]";
    server.send(200, "application/json", historyJson);
  });

  server.begin();  // Start the server
  Serial.println("Web server started!");
}

void loop() {
  // Simulated reading and logging every 5 seconds
  static unsigned long lastLogTime = 0;
  if (millis() - lastLogTime > 5000) {
    lastLogTime = millis();
    String ecValue = String(random(500, 1500) / 100.0);  // Simulated EC value
    String pHValue = String(random(40, 80) / 10.0);      // Simulated pH value
    String timeValue = String(millis() / 1000) + "s";    // Simulated timestamp

    ecLog[currentIndex] = ecValue;
    pHLog[currentIndex] = pHValue;
    timeLog[currentIndex] = timeValue;
    currentIndex = (currentIndex + 1) % MAX_ENTRIES;

    Serial.println("Logged: " + ecValue + " mS/cm, " + pHValue + " pH");
  }

  server.handleClient();  // Handle client requests
}
