#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>

const char* ssid = "Tic Center";
const char* password = "05022002";

WebServer server(80);

// Sensor setup - Three sensors
const int sensorPin1 = 35;  // First moisture sensor
const int sensorPin2 = 34;  // Second moisture sensor
const int sensorPin3 = 32;  // Third moisture sensor (changed from 35 duplicate)
const int pumpPin = 17;     // Pump control pin (Wemos D1 mini)
const int AirValue = 2300;  // Reading in air (dry)
const int WaterValue = 1000; // Reading in water (wet)

// Mode: 0 = Manual, 1 = Automatic
int mode = 0;
bool manualPumpState = false;
int autoThresholdLow = 30;   // Start pumping at this moisture %
int autoThresholdHigh = 90;  // Stop pumping at this moisture %
bool pumpActive = false;

// EEPROM addresses
#define EEPROM_MODE_ADDR 0
#define EEPROM_LOW_ADDR 1
#define EEPROM_HIGH_ADDR 2

void saveModeToEEPROM() {
  EEPROM.write(EEPROM_MODE_ADDR, mode);
  EEPROM.commit();
}

void saveThresholdsToEEPROM() {
  EEPROM.write(EEPROM_LOW_ADDR, autoThresholdLow);
  EEPROM.write(EEPROM_HIGH_ADDR, autoThresholdHigh);
  EEPROM.commit();
}

void loadSettingsFromEEPROM() {
  mode = EEPROM.read(EEPROM_MODE_ADDR);
  autoThresholdLow = EEPROM.read(EEPROM_LOW_ADDR);
  autoThresholdHigh = EEPROM.read(EEPROM_HIGH_ADDR);
  
  if (autoThresholdLow == 255) autoThresholdLow = 30;
  if (autoThresholdHigh == 255) autoThresholdHigh = 90;
}

void updatePump(int currentMoisture) {
  if (mode == 0) {
    // Manual mode
    digitalWrite(pumpPin, manualPumpState ? HIGH : LOW);
    pumpActive = manualPumpState;
  } else {
    // Automatic mode - using average of all sensors
    if (currentMoisture <= autoThresholdLow && !pumpActive) {
      digitalWrite(pumpPin, HIGH);
      pumpActive = true;
    } else if (currentMoisture >= autoThresholdHigh && pumpActive) {
      digitalWrite(pumpPin, LOW);
      pumpActive = false;
    }
  }
}

String htmlPage() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Smart Agriculture | Soil Moisture</title>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <style>
        body {
          margin: 0;
          font-family: 'Poppins', sans-serif;
          background: linear-gradient(135deg, #0a192f, #0f4c75, #1b262c);
          color: #eafaf1;
          text-align: center;
          min-height: 100vh;
          overflow-x: hidden;
          display: flex;
          flex-direction: column;
          justify-content: center;
          align-items: center;
          padding: 20px;
        }

        .logo {
          max-width: 150px;
          width: 150px;
          height: 150px;
          border-radius: 12px;
          box-shadow: 0 0 15px rgba(0, 255, 200, 0.4);
          margin-bottom: 20px;
          transition: transform 0.3s ease, box-shadow 0.4s ease-in-out;
          object-fit: cover;
          background-color: rgba(255, 255, 255, 0.05);
          padding: 8px;
          flex-shrink: 0;
        }

        .logo:hover {
          transform: scale(1.05);
          box-shadow: 0 0 25px rgba(0, 255, 200, 0.8);
        }

        .glow {
          box-shadow: 0 0 35px rgba(0, 255, 200, 0.9);
        }

        .main-container {
          display: flex;
          gap: 20px;
          justify-content: center;
          align-items: flex-start;
          max-width: 1400px;
          flex-wrap: wrap;
        }

        .card {
          background: rgba(255, 255, 255, 0.08);
          border: 1px solid rgba(0, 255, 200, 0.25);
          border-radius: 20px;
          padding: 40px;
          width: 320px;
          box-shadow: 0 8px 25px rgba(0, 0, 0, 0.4);
          backdrop-filter: blur(8px);
        }

        .control-card {
          background: rgba(255, 255, 255, 0.08);
          border: 1px solid rgba(0, 255, 200, 0.25);
          border-radius: 20px;
          padding: 30px;
          width: 320px;
          box-shadow: 0 8px 25px rgba(0, 0, 0, 0.4);
          backdrop-filter: blur(8px);
        }

        h1 {
          font-size: 1.5em;
          color: #9be7ff;
          margin-bottom: 25px;
          letter-spacing: 1px;
        }

        h2 {
          font-size: 1.2em;
          color: #9be7ff;
          margin-bottom: 20px;
          letter-spacing: 1px;
        }

        .sensor-tabs {
          display: flex;
          gap: 8px;
          margin-bottom: 20px;
          justify-content: center;
        }

        .sensor-tab {
          flex: 1;
          padding: 10px;
          border: 2px solid rgba(0, 255, 200, 0.3);
          background: rgba(0, 255, 200, 0.1);
          color: #9be7ff;
          border-radius: 8px;
          cursor: pointer;
          font-size: 0.85em;
          font-weight: 600;
          transition: all 0.3s ease;
        }

        .sensor-tab.active {
          background: rgba(0, 255, 200, 0.4);
          border-color: #00e5ff;
          box-shadow: 0 0 15px rgba(0, 255, 200, 0.6);
        }

        .sensor-tab:hover {
          border-color: #00e5ff;
          background: rgba(0, 255, 200, 0.2);
        }

        .container {
          position: relative;
          width: 180px;
          height: 180px;
          margin: 0 auto 20px;
          border-radius: 50%;
          overflow: hidden;
          background: #004d40;
          border: 4px solid #00bcd4;
          box-shadow: inset 0 0 20px rgba(0, 188, 212, 0.5);
        }

        .wave {
          position: absolute;
          bottom: 0;
          width: 200%;
          height: 200%;
          background: linear-gradient(180deg, #00e5ff 0%, #00bfa5 100%);
          border-radius: 35%;
          animation: waveAnimation 4s infinite linear;
          transform: translateX(-25%);
          transition: top 0.8s ease-in-out;
        }

        @keyframes waveAnimation {
          from { transform: translateX(-25%) rotate(0deg); }
          to { transform: translateX(-25%) rotate(360deg); }
        }

        .value {
          position: absolute;
          top: 50%;
          left: 50%;
          transform: translate(-50%, -50%);
          font-size: 2.5em;
          font-weight: 600;
          color: #e0f7fa;
          z-index: 10;
        }

        .sensor-label {
          position: absolute;
          top: 15px;
          left: 50%;
          transform: translateX(-50%);
          font-size: 0.8em;
          color: #81d4fa;
          font-weight: 600;
          z-index: 10;
        }

        p {
          color: #a5d6a7;
          font-size: 1em;
          margin-top: 15px;
        }

        .footer {
          margin-top: 20px;
          font-size: 0.8em;
          color: #81d4fa;
        }

        .mode-buttons {
          display: flex;
          gap: 10px;
          margin-bottom: 20px;
        }

        .mode-btn {
          flex: 1;
          padding: 12px;
          border: 2px solid rgba(0, 255, 200, 0.5);
          background: rgba(0, 255, 200, 0.1);
          color: #9be7ff;
          border-radius: 10px;
          cursor: pointer;
          font-size: 0.9em;
          font-weight: 600;
          transition: all 0.3s ease;
        }

        .mode-btn.active {
          background: rgba(0, 255, 200, 0.4);
          border-color: #00e5ff;
          box-shadow: 0 0 15px rgba(0, 255, 200, 0.6);
        }

        .mode-btn:hover {
          border-color: #00e5ff;
          background: rgba(0, 255, 200, 0.2);
        }

        .pump-toggle {
          width: 100%;
          padding: 15px;
          margin: 15px 0;
          border: none;
          background: linear-gradient(135deg, #00e5ff, #00bfa5);
          color: #0a192f;
          border-radius: 10px;
          font-size: 1.1em;
          font-weight: 700;
          cursor: pointer;
          transition: all 0.3s ease;
          box-shadow: 0 4px 15px rgba(0, 229, 255, 0.4);
        }

        .pump-toggle:hover {
          transform: translateY(-2px);
          box-shadow: 0 6px 20px rgba(0, 229, 255, 0.6);
        }

        .pump-toggle.off {
          background: linear-gradient(135deg, #666, #444);
          box-shadow: 0 4px 15px rgba(100, 100, 100, 0.4);
        }

        .threshold-group {
          margin: 15px 0;
          text-align: left;
        }

        .threshold-group label {
          display: block;
          color: #a5d6a7;
          font-size: 0.9em;
          margin-bottom: 8px;
          font-weight: 600;
        }

        .threshold-group input {
          width: 100%;
          padding: 10px;
          border: 1px solid rgba(0, 255, 200, 0.3);
          background: rgba(255, 255, 255, 0.05);
          color: #eafaf1;
          border-radius: 8px;
          font-size: 1em;
          box-sizing: border-box;
        }

        .threshold-group input:focus {
          outline: none;
          border-color: #00e5ff;
          box-shadow: 0 0 10px rgba(0, 229, 255, 0.3);
        }

        .save-btn {
          width: 100%;
          padding: 10px;
          margin-top: 10px;
          border: none;
          background: rgba(0, 255, 200, 0.3);
          color: #a5d6a7;
          border-radius: 8px;
          cursor: pointer;
          font-size: 0.9em;
          transition: all 0.3s ease;
        }

        .save-btn:hover {
          background: rgba(0, 255, 200, 0.5);
          color: #eafaf1;
        }

        .pump-status {
          padding: 10px;
          margin-top: 15px;
          border-radius: 8px;
          font-size: 0.95em;
          font-weight: 600;
        }

        .pump-status.active {
          background: rgba(0, 229, 255, 0.2);
          color: #00e5ff;
          border: 1px solid rgba(0, 229, 255, 0.5);
        }

        .pump-status.inactive {
          background: rgba(100, 100, 100, 0.2);
          color: #999;
          border: 1px solid rgba(100, 100, 100, 0.5);
        }

        .sensor-info {
          display: flex;
          justify-content: space-around;
          margin-top: 15px;
          padding: 15px;
          background: rgba(0, 0, 0, 0.2);
          border-radius: 10px;
        }

        .sensor-mini {
          text-align: center;
        }

        .sensor-mini-value {
          font-size: 1.3em;
          font-weight: 600;
          color: #00e5ff;
        }

        .sensor-mini-label {
          font-size: 0.75em;
          color: #81d4fa;
          margin-top: 5px;
        }
      </style>

      <script>
        let currentMoisture = [0, 0, 0];
        let activeSensor = 0;
        let mode = 0;
        let pumpActive = false;
        let autoLow = 30;
        let autoHigh = 90;

        function updateData() {
          fetch('/status')
            .then(response => response.json())
            .then(data => {
              currentMoisture[0] = data.moisture1;
              currentMoisture[1] = data.moisture2;
              currentMoisture[2] = data.moisture3;
              mode = data.mode;
              pumpActive = data.pumpActive;
              autoLow = data.autoLow;
              autoHigh = data.autoHigh;

              updateDisplay();

              const logo = document.getElementById('logo');
              logo.classList.add('glow');
              setTimeout(() => logo.classList.remove('glow'), 500);
            });
        }

        function updateDisplay() {
          document.getElementById('moistureValue').innerText = currentMoisture[activeSensor] + '%';
          document.querySelector('.wave').style.top = (100 - currentMoisture[activeSensor]) + '%';
          document.getElementById('sensorLabel').innerText = 'Sensor ' + (activeSensor + 1);

          // Update mini displays
          document.getElementById('mini1').innerText = currentMoisture[0] + '%';
          document.getElementById('mini2').innerText = currentMoisture[1] + '%';
          document.getElementById('mini3').innerText = currentMoisture[2] + '%';

          updateUI();
        }

        function switchSensor(index) {
          activeSensor = index;
          
          // Update tab styling
          document.querySelectorAll('.sensor-tab').forEach((tab, i) => {
            if (i === index) {
              tab.classList.add('active');
            } else {
              tab.classList.remove('active');
            }
          });

          updateDisplay();
        }

        function updateUI() {
          const manualBtn = document.getElementById('manualBtn');
          const autoBtn = document.getElementById('autoBtn');
          const toggleBtn = document.getElementById('toggleBtn');
          const statusDiv = document.getElementById('pumpStatus');
          const autoControls = document.getElementById('autoControls');

          if (mode === 0) {
            manualBtn.classList.add('active');
            autoBtn.classList.remove('active');
            autoControls.style.display = 'none';
            toggleBtn.style.display = 'block';
          } else {
            autoBtn.classList.add('active');
            manualBtn.classList.remove('active');
            autoControls.style.display = 'block';
            toggleBtn.style.display = 'none';
          }

          if (pumpActive) {
            toggleBtn.classList.remove('off');
            toggleBtn.innerText = 'PUMP ON';
            statusDiv.classList.add('active');
            statusDiv.classList.remove('inactive');
            statusDiv.innerText = 'Pump is RUNNING';
          } else {
            toggleBtn.classList.add('off');
            toggleBtn.innerText = 'PUMP OFF';
            statusDiv.classList.remove('active');
            statusDiv.classList.add('inactive');
            statusDiv.innerText = 'Pump is OFF';
          }

          document.getElementById('autoLowInput').value = autoLow;
          document.getElementById('autoHighInput').value = autoHigh;
        }

        function setMode(newMode) {
          fetch('/setMode', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ mode: newMode })
          }).then(() => updateData());
        }

        function togglePump() {
          fetch('/togglePump', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' }
          }).then(() => updateData());
        }

        function saveThresholds() {
          const low = parseInt(document.getElementById('autoLowInput').value);
          const high = parseInt(document.getElementById('autoHighInput').value);

          if (low >= high) {
            alert('Low threshold must be less than high threshold!');
            return;
          }
          if (low < 0 || high > 100) {
            alert('Values must be between 0 and 100!');
            return;
          }

          fetch('/setThresholds', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ low: low, high: high })
          }).then(() => {
            alert('Thresholds saved!');
            updateData();
          });
        }

        setInterval(updateData, 2000);
        window.onload = updateData;
      </script>
    </head>
    <body>
      <img id="logo" class="logo" src="https://scontent.fcai19-8.fna.fbcdn.net/v/t39.30808-6/306283594_516673790462572_392324543426136387_n.jpg?_nc_cat=110&ccb=1-7&_nc_sid=6ee11a&_nc_eui2=AeF1BjNjviWoOSuqWtjyuUmIIz6oGHfCGgkjPqgYd8IaCfTSfr4KtxwFASeHI4MDds9SOFm6gnyh8dm0w2m1v_Sb&_nc_ohc=Sne_Qc0guwsQ7kNvwG1xHL4&_nc_oc=AdmS7a1aSdpPMVdUUId5ElgyyCVudwTDVxIBEC3pi9LbC5xj2-t75RYeHiWRu5UrecM&_nc_zt=23&_nc_ht=scontent.fcai19-8.fna&_nc_gid=LqIlgeqE08lIiBHFr9V2EA&oh=00_AfeLEhFAtsGKGf5GfYFqUYhkGMSE47LTSBdWXndwgPRe-g&oe=690842D1" alt="Smart Agriculture Logo">

      <div class="main-container">
        <div class="card">
          <h1>Soil Moisture Level</h1>
          
          <div class="sensor-tabs">
            <button class="sensor-tab active" onclick="switchSensor(0)">Sensor 1</button>
            <button class="sensor-tab" onclick="switchSensor(1)">Sensor 2</button>
            <button class="sensor-tab" onclick="switchSensor(2)">Sensor 3</button>
          </div>

          <div class="container">
            <div class="sensor-label" id="sensorLabel">Sensor 1</div>
            <div class="wave" id="wave"></div>
            <div class="value" id="moistureValue">--%</div>
          </div>
          
          <div class="sensor-info">
            <div class="sensor-mini">
              <div class="sensor-mini-value" id="mini1">--%</div>
              <div class="sensor-mini-label">S1</div>
            </div>
            <div class="sensor-mini">
              <div class="sensor-mini-value" id="mini2">--%</div>
              <div class="sensor-mini-label">S2</div>
            </div>
            <div class="sensor-mini">
              <div class="sensor-mini-value" id="mini3">--%</div>
              <div class="sensor-mini-label">S3</div>
            </div>
          </div>

          <p>Live data from ESP32</p>
          <div class="footer">Smart Agriculture System</div>
        </div>

        <div class="control-card">
          <h2>Pump Control</h2>
          
          <div class="mode-buttons">
            <button class="mode-btn active" id="manualBtn" onclick="setMode(0)">Manual</button>
            <button class="mode-btn" id="autoBtn" onclick="setMode(1)">Automatic</button>
          </div>

          <button class="pump-toggle" id="toggleBtn" onclick="togglePump()">PUMP OFF</button>
          
          <div id="pumpStatus" class="pump-status inactive">Pump is OFF</div>

          <div id="autoControls" style="display: none;">
            <div class="threshold-group">
              <label>Start Pumping At (%):</label>
              <input type="number" id="autoLowInput" min="0" max="100" value="30">
            </div>
            <div class="threshold-group">
              <label>Stop Pumping At (%):</label>
              <input type="number" id="autoHighInput" min="0" max="100" value="90">
            </div>
            <button class="save-btn" onclick="saveThresholds()">Save Thresholds</button>
          </div>
        </div>
      </div>
    </body>
    </html>
  )rawliteral";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleStatus() {
  int sensorValue1 = analogRead(sensorPin1);
  int sensorValue2 = analogRead(sensorPin2);
  int sensorValue3 = analogRead(sensorPin3);
  
  int moisture1 = map(sensorValue1, AirValue, WaterValue, 0, 100);
  int moisture2 = map(sensorValue2, AirValue, WaterValue, 0, 100);
  int moisture3 = map(sensorValue3, AirValue, WaterValue, 0, 100);
  
  moisture1 = constrain(moisture1, 0, 100);
  moisture2 = constrain(moisture2, 0, 100);
  moisture3 = constrain(moisture3, 0, 100);

  String json = "{\"moisture1\": " + String(moisture1) + 
                ", \"moisture2\": " + String(moisture2) + 
                ", \"moisture3\": " + String(moisture3) + 
                ", \"mode\": " + String(mode) + 
                ", \"pumpActive\": " + (pumpActive ? String("true") : String("false")) +
                ", \"autoLow\": " + String(autoThresholdLow) +
                ", \"autoHigh\": " + String(autoThresholdHigh) + "}";
  server.send(200, "application/json", json);
}

void handleSetMode() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    int newMode = body.indexOf("\"mode\":0") != -1 ? 0 : 1;
    mode = newMode;
    saveModeToEEPROM();
    
    if (mode == 0) {
      manualPumpState = false;
      digitalWrite(pumpPin, LOW);
      pumpActive = false;
    }
    
    server.send(200, "application/json", "{\"status\": \"ok\"}");
  }
}

void handleTogglePump() {
  if (mode == 0) {
    manualPumpState = !manualPumpState;
    digitalWrite(pumpPin, manualPumpState ? HIGH : LOW);
    pumpActive = manualPumpState;
  }
  server.send(200, "application/json", "{\"status\": \"ok\"}");
}

void handleSetThresholds() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    int lowIdx = body.indexOf("\"low\":") + 6;
    int highIdx = body.indexOf("\"high\":") + 7;
    
    autoThresholdLow = body.substring(lowIdx, body.indexOf(",", lowIdx)).toInt();
    autoThresholdHigh = body.substring(highIdx, body.indexOf("}", highIdx)).toInt();
    
    saveThresholdsToEEPROM();
    server.send(200, "application/json", "{\"status\": \"ok\"}");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);

  EEPROM.begin(512);
  loadSettingsFromEEPROM();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/setMode", handleSetMode);
  server.on("/togglePump", handleTogglePump);
  server.on("/setThresholds", handleSetThresholds);
  server.begin();
  Serial.println("Web server started!");
}

void loop() {
  server.handleClient();

  int sensorValue1 = analogRead(sensorPin1);
  int sensorValue2 = analogRead(sensorPin2);
  int sensorValue3 = analogRead(sensorPin3);
  
  int moisture1 = map(sensorValue1, AirValue, WaterValue, 0, 100);
  int moisture2 = map(sensorValue2, AirValue, WaterValue, 0, 100);
  int moisture3 = map(sensorValue3, AirValue, WaterValue, 0, 100);
  
  moisture1 = constrain(moisture1, 0, 100);
  moisture2 = constrain(moisture2, 0, 100);
  moisture3 = constrain(moisture3, 0, 100);
  
  // Use average of all sensors for automatic mode
  int avgMoisture = (moisture1 + moisture2 + moisture3) / 3;
  updatePump(avgMoisture);

  Serial.print("S1: ");
  Serial.print(moisture1);
  Serial.print("% | S2: ");
  Serial.print(moisture2);
  Serial.print("% | S3: ");
  Serial.print(moisture3);
  Serial.print("% | Avg: ");
  Serial.print(avgMoisture);
  Serial.print("% | Mode: ");
  Serial.print(mode == 0 ? "Manual" : "Automatic");
  Serial.print(" | Pump: ");
  Serial.println(pumpActive ? "ON" : "OFF");

  delay(500);
}