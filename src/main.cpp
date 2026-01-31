#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi credentials - UPDATE THESE WITH YOUR NETWORK DETAILS
const char* ssid = "AK's Weapon";
const char* password = "1122334455";

// Ultrasonic sensor pins
#define TRIG_PIN 5
#define ECHO_PIN 18

// Bin dimensions (in cm) - UPDATE THESE TO MATCH YOUR BIN
#define BIN_HEIGHT 100.0  // Total height of your bin in cm
#define SENSOR_OFFSET 5.0 // Distance from sensor to bin top in cm

// Web server on port 80
WebServer server(80);

long duration;
float distance;
float fillPercentage;

// Variables to store readings for averaging
const int numReadings = 5;
float readings[numReadings];
int readIndex = 0;
float total = 0;
float average = 0;

// Function to get distance reading
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH, 30000);
  float dist = duration * 0.034 / 2;
  
  return dist;
}

// Function to calculate fill percentage
float calculateFillPercentage(float dist) {
  if (dist == 0 || dist > BIN_HEIGHT + SENSOR_OFFSET) {
    return 0.0;  // Out of range or error
  }
  
  // Calculate empty space from sensor
  float emptySpace = dist - SENSOR_OFFSET;
  
  // Calculate fill percentage
  float fillPct = ((BIN_HEIGHT - emptySpace) / BIN_HEIGHT) * 100.0;
  
  // Constrain between 0 and 100
  if (fillPct < 0) fillPct = 0;
  if (fillPct > 100) fillPct = 100;
  
  return fillPct;
}

// Handle root request - serves the dashboard HTML
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>EmptyBin Monitor</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', system-ui, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        
        .container {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 24px;
            padding: 40px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            max-width: 500px;
            width: 100%;
        }
        
        h1 {
            color: #2d3748;
            font-size: 32px;
            margin-bottom: 10px;
            text-align: center;
        }
        
        .subtitle {
            color: #718096;
            text-align: center;
            margin-bottom: 40px;
            font-size: 14px;
        }
        
        .bin-container {
            position: relative;
            width: 200px;
            height: 300px;
            margin: 0 auto 30px;
            background: #f7fafc;
            border: 4px solid #4a5568;
            border-radius: 12px;
            overflow: hidden;
        }
        
        .bin-fill {
            position: absolute;
            bottom: 0;
            width: 100%;
            background: linear-gradient(180deg, #48bb78 0%, #38a169 100%);
            transition: height 0.5s ease-out, background 0.5s ease-out;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: bold;
            font-size: 24px;
        }
        
        .stats {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
            margin-top: 30px;
        }
        
        .stat-card {
            background: #f7fafc;
            padding: 20px;
            border-radius: 12px;
            text-align: center;
        }
        
        .stat-label {
            color: #718096;
            font-size: 12px;
            text-transform: uppercase;
            margin-bottom: 8px;
            font-weight: 600;
            letter-spacing: 0.5px;
        }
        
        .stat-value {
            color: #2d3748;
            font-size: 24px;
            font-weight: bold;
        }
        
        .status {
            text-align: center;
            margin-top: 20px;
            padding: 12px;
            border-radius: 8px;
            font-weight: 600;
        }
        
        .status.ok {
            background: #c6f6d5;
            color: #22543d;
        }
        
        .status.warning {
            background: #feebc8;
            color: #7c2d12;
        }
        
        .status.full {
            background: #fed7d7;
            color: #742a2a;
        }
        
        .last-update {
            text-align: center;
            color: #a0aec0;
            font-size: 12px;
            margin-top: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🗑️ EmptyBin Monitor</h1>
        <p class="subtitle">Real-time waste bin level tracking</p>
        
        <div class="bin-container">
            <div class="bin-fill" id="binFill">
                <span id="percentage">0%</span>
            </div>
        </div>
        
        <div class="stats">
            <div class="stat-card">
                <div class="stat-label">Fill Level</div>
                <div class="stat-value" id="fillValue">0%</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Distance</div>
                <div class="stat-value" id="distValue">-- cm</div>
            </div>
        </div>
        
        <div class="status ok" id="status">Loading...</div>
        
        <div class="last-update" id="lastUpdate">Connecting...</div>
    </div>
    
    <script>
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    const fillPct = Math.round(data.fillPercentage);
                    const distance = data.distance.toFixed(2);
                    
                    // Update bin visualization
                    document.getElementById('binFill').style.height = fillPct + '%';
                    document.getElementById('percentage').textContent = fillPct + '%';
                    
                    // Update stats
                    document.getElementById('fillValue').textContent = fillPct + '%';
                    document.getElementById('distValue').textContent = distance + ' cm';
                    
                    // Update status
                    const statusDiv = document.getElementById('status');
                    if (fillPct < 60) {
                        statusDiv.className = 'status ok';
                        statusDiv.textContent = '✓ Bin has plenty of space';
                    } else if (fillPct < 85) {
                        statusDiv.className = 'status warning';
                        statusDiv.textContent = '⚠ Bin getting full - consider emptying soon';
                    } else {
                        statusDiv.className = 'status full';
                        statusDiv.textContent = '⚠ Bin nearly full - empty immediately!';
                    }
                    
                    // Update bin fill color based on level
                    const binFill = document.getElementById('binFill');
                    if (fillPct < 60) {
                        binFill.style.background = 'linear-gradient(180deg, #48bb78 0%, #38a169 100%)';
                    } else if (fillPct < 85) {
                        binFill.style.background = 'linear-gradient(180deg, #ecc94b 0%, #d69e2e 100%)';
                    } else {
                        binFill.style.background = 'linear-gradient(180deg, #fc8181 0%, #f56565 100%)';
                    }
                    
                    // Update last update time
                    const now = new Date();
                    document.getElementById('lastUpdate').textContent = 
                        'Last updated: ' + now.toLocaleTimeString();
                })
                .catch(error => {
                    console.error('Error fetching data:', error);
                    document.getElementById('status').textContent = '⚠ Connection error';
                });
        }
        
        // Update every 2 seconds
        updateData();
        setInterval(updateData, 2000);
    </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

// Handle data request - returns JSON with sensor data
void handleData() {
  // Get current distance
  distance = getDistance();
  
  // Calculate fill percentage
  fillPercentage = calculateFillPercentage(distance);
  
  // Create JSON response
  StaticJsonDocument<200> doc;
  doc["distance"] = distance;
  doc["fillPercentage"] = fillPercentage;
  doc["timestamp"] = millis();
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
}

void setup() {
  Serial.begin(115200);
  
  // Setup sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Initialize readings array
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0;
  }
  
  Serial.println("\n\nEmptyBin Monitor Starting...");
  Serial.println("================================");
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Access dashboard at: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n✗ WiFi Connection Failed!");
    Serial.println("Please check your credentials and try again.");
  }
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  
  // Start server
  server.begin();
  Serial.println("Web server started!");
  Serial.println("================================\n");
}

void loop() {
  // Handle web server requests
  server.handleClient();
  
  // Get distance reading
  distance = getDistance();
  
  // Smooth the readings using moving average
  total = total - readings[readIndex];
  readings[readIndex] = distance;
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % numReadings;
  average = total / numReadings;
  
  // Calculate fill percentage
  fillPercentage = calculateFillPercentage(average);
  
  // Print to serial monitor
  if (distance == 0) {
    Serial.println("Out of range");
  } else {
    Serial.print("Distance: ");
    Serial.print(average, 2);
    Serial.print(" cm | Fill: ");
    Serial.print(fillPercentage, 1);
    Serial.println("%");
  }
  
  delay(1000);
}
