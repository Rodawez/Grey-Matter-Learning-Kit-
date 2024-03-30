#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h" // Include DHT sensor library
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS   1000
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define WIFI_SSID "Brain"
#define WIFI_PSWD "Brain123"
#define LED_PIN 2
#define NUM_LEDS 24
#define DHT_PIN 14 // Define the pin for the DHT sensor
#define DHT_TYPE DHT11 // Define the type of DHT sensor
#define PULSE_SENSOR_PIN A0 

Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
ESP8266WebServer server(80);
DHT dht(DHT_PIN, DHT_TYPE); // Initialize the DHT sensor
int Signal;         // Store incoming ADC data. Value can range from 0-1024
int Threshold = 550; 

const char* texts[5] = {
  "Temporal  Lobe",
  "Cerebellum",
  "Occipital Lobe",
  "Parietal  Lobe",
  "Frontal   Lobe"
};

const uint32_t colors[] = {
  pixels.Color(255, 255, 0),    // Yellow
  pixels.Color(255, 0, 0),      // Red
  pixels.Color(0, 255, 0),      // Green
  pixels.Color(0, 0, 255),      // Blue
  pixels.Color(255, 165, 0),    // Orange
  pixels.Color(148, 0, 211),    // Violet
  pixels.Color(0, 191, 255),    // Sky Blue
  pixels.Color(255, 192, 203)   // Pink
};

bool animationRunning = false;
bool temporalLobeEnabled = false;
bool cerebellumEnabled = false;
bool occipitalLobeEnabled = false;
bool parietalLobeEnabled = false;
bool frontalLobeEnabled = false;

void handleRoot() {
  String html = "<html><head>";
  html += "<style>";
  // CSS for button and animation
  html += ".button {";
  html += "  background-color: #90EE90;";
  html += "  display: inline-block;";
  html += "  padding: 10px 20px;";
  html += "  margin: 5px;"; // Add margin for spacing between buttons
  html += "  font-size: 16px;";
  html += "  cursor: pointer;";
  html += "  text-align: center;";
  html += "  text-decoration: none;";
  html += "  outline: none;";
  html += "  color: #fff;";
  html += "  border: none;";
  html += "  border-radius: 15px;";
  html += "  box-shadow: 0 9px #999;";
  html += "}";
  html += ".button:hover {";
  html += "  background-color: #3e8e41;";
  html += "}";
  html += ".button:active {";
  html += "  background-color: #3e8e41;";
  html += "  box-shadow: 0 5px #666;";
  html += "  transform: translateY(4px);";
  html += "}";
  html += ".button-container {";
  html += "  display: flex;";
  html += "  flex-wrap: wrap;"; // Allow buttons to wrap to the next line if necessary
  html += "}";
  html += "</style>";
  html += "</head><body>";
  
  html += "<div class=\"button-container\">"; // Container for buttons
  
  // HTML for the Brain Status button
  html += "<button class=\"button\" onclick=\"toggleAnimation()\" id=\"Brain\">Brain Status</button>";
  
  // HTML for the Temporal Lobe button
  html += "<button class=\"button\" onclick=\"toggleTemporalLobe()\" id=\"TemporalLobe\">Temporal Lobe</button>";
  
  // HTML for the Cerebellum button
  html += "<button class=\"button\" onclick=\"toggleCerebellum()\" id=\"Cerebellum\">Cerebellum</button>";
  
  // HTML for the Occipital Lobe button
  html += "<button class=\"button\" onclick=\"toggleOccipitalLobe()\" id=\"OccipitalLobe\">Occipital Lobe</button>";
  
  // HTML for the Parietal Lobe button
  html += "<button class=\"button\" onclick=\"toggleParietalLobe()\" id=\"ParietalLobe\">Parietal Lobe</button>";
  
  // HTML for the Frontal Lobe button
  html += "<button class=\"button\" onclick=\"toggleFrontalLobe()\" id=\"FrontalLobe\">Frontal Lobe</button>";
  
  html += "</div>"; // End of button container

  // New buttons for sensors
  html += "<div class=\"button-container\">";
  html += "<button class=\"button\" onclick=\"toggleSensors()\" id=\"Sensors\">Sensors</button>";
  html += "</div>";
  html += "<div class=\"sensor-data\">";
  html += "<h2>Sensor Data</h2>";
  html += "<p>Temperature: " + String(temp) + " °C</p>";
  html += "<p>Humidity: " + String(hum) + " %</p>";
  html += "</div>";

 
  
  html += "<script>";
  // JavaScript function to toggle the animation and button color
  html += "function toggleAnimation() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      if (this.responseText == 'Turn On') {";
  html += "        document.getElementById('Brain').style.backgroundColor = 'blue';";
  html += "      } else if (this.responseText == 'Turn Off') {";
  html += "        document.getElementById('Brain').style.backgroundColor = 'red';";
  html += "      }";
  html += "    }";
  html += "  };";
  html += "  xhr.open('GET', '/toggle', true);";
  html += "  xhr.send();";
  html += "}";
  
  // JavaScript function to toggle the Temporal Lobe logic
  html += "function toggleTemporalLobe() {";
  html += "  document.getElementById('TemporalLobe').style.backgroundColor = 'blue';"; // Change button color to blue
  
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      document.getElementById('TemporalLobe').style.backgroundColor = 'green';"; // Change button color to green
  html += "    }";
  html += "  };";
  html += "  xhr.open('GET', '/temporallobe', true);";
  html += "  xhr.send();";
  html += "}";
  
  // JavaScript function to toggle the Cerebellum logic
  html += "function toggleCerebellum() {";
  html += "  document.getElementById('Cerebellum').style.backgroundColor = 'blue';"; // Change button color to blue
  
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      document.getElementById('Cerebellum').style.backgroundColor = 'green';"; // Change button color to green
  html += "    }";
  html += "  };";
  html += "  xhr.open('GET', '/cerebellum', true);";
  html += "  xhr.send();";
  html += "}";
  
  // JavaScript function to toggle the Occipital Lobe logic
  html += "function toggleOccipitalLobe() {";
  html += "  document.getElementById('OccipitalLobe').style.backgroundColor = 'blue';"; // Change button color to blue
  
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      document.getElementById('OccipitalLobe').style.backgroundColor = 'green';"; // Change button color to green
  html += "    }";
  html += "  };";
  html += "  xhr.open('GET', '/occipitallobe', true);";
  html += "  xhr.send();";
  html += "}";
  
  // JavaScript function to toggle the Parietal Lobe logic
  html += "function toggleParietalLobe() {";
  html += "  document.getElementById('ParietalLobe').style.backgroundColor = 'blue';"; // Change button color to blue
  
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      document.getElementById('ParietalLobe').style.backgroundColor = 'green';"; // Change button color to green
  html += "    }";
  html += "  };";
  html += "  xhr.open('GET', '/parietallobe', true);";
  html += "  xhr.send();";
  html += "}";
  
  // JavaScript function to toggle the Frontal Lobe logic
  html += "function toggleFrontalLobe() {";
  html += "  document.getElementById('FrontalLobe').style.backgroundColor = 'blue';"; // Change button color to blue
  
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      document.getElementById('FrontalLobe').style.backgroundColor = 'green';"; // Change button color to green
  html += "    }";
  html += "  };";
  html += "  xhr.open('GET', '/frontallobe', true);";
  html += "  xhr.send();";
  html += "}";
  
  // JavaScript function to toggle the Sensors
  html += "function toggleSensors() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      document.getElementById('Sensors').style.backgroundColor = 'green';"; // Change button color to green
  html += "    }";
  html += "  };";
  html += "  xhr.open('GET', '/sensors', true);";
  html += "  xhr.send();";
  html += "}";  
  html += "</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}
void handleToggle() {
  animationRunning = !animationRunning;
  
  if (animationRunning) {
    for (int i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
      for (int j = 0; j < NUM_LEDS; j++) {
        pixels.setPixelColor(j, colors[i]);
      }
      pixels.show();
      delay(100);
      for (int j = 0; j < NUM_LEDS; j++) {
        pixels.setPixelColor(j, pixels.Color(0, 0, 0));
      }
      pixels.show();
      delay(500);
    }

    for (int j = 0; j < NUM_LEDS; j++) {
      pixels.setPixelColor(j, colors[0]);
    }
    pixels.show();

    server.send(200, "text/plain", "Turn Off");
  } else {
    for (int i = 0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();

    server.send(200, "text/plain", "Turn On");
  }
}

void handleTemporalLobe() {
  if (animationRunning) {
    // Insert logic for Temporal Lobe LED configuration here
    for (int j = 0; j < NUM_LEDS; j++) {
      pixels.setPixelColor(j, pixels.Color(0, 0, 0));
    }

    // Replace these LED configurations with the specific LED configuration for the Parietal Lobe
    // For the sake of this example, I'll use the same LED configurations as the Occipital Lobe
    for (int j = 11; j < 13; j++) {
      pixels.setPixelColor(j, colors[2]);
    }
   
    pixels.show();
    display.clearDisplay();

    display.setCursor(0, 0);
    display.setTextSize(2);
    display.println(texts[0]);
    display.display();
    delay(3000);
    display.clearDisplay();
    display.display();
  
    

    for (int j = 0; j < NUM_LEDS; j++) {
      pixels.setPixelColor(j, colors[0]);
    }
    pixels.show();
    server.send(200, "text/plain", "Temporal Lobe action executed");
  } else {
    server.send(200, "text/plain", "Animation is not running");
  }
}

void handleCerebellum() {
  if (animationRunning) {
    // Insert logic for Cerebellum LED configuration here
    for (int j = 0; j < NUM_LEDS; j++) {
      pixels.setPixelColor(j, pixels.Color(0, 0, 0));
    }

    // Replace these LED configurations with the specific LED configuration for the Parietal Lobe
    // For the sake of this example, I'll use the same LED configurations as the Occipital Lobe
   
    for (int j = 16; j < 18; j++) {
      pixels.setPixelColor(j, colors[3]);
    }
    pixels.show();
    display.clearDisplay();

    display.setCursor(0, 0);
    display.setTextSize(2);
    display.println(texts[1]);
    display.display();
    delay(3000);
    display.clearDisplay();
    display.display();

    for (int j = 0; j < NUM_LEDS; j++) {
      pixels.setPixelColor(j, colors[0]);
    }
    pixels.show();
    server.send(200, "text/plain", "Cerebellum action executed");
  } else {
    server.send(200, "text/plain", "Animation is not running");
  }
}

void handleOccipitalLobe() {
  if (animationRunning) {
    // Insert logic for Occipital Lobe LED configuration here
    for (int j = 0; j < NUM_LEDS; j++) {
      pixels.setPixelColor(j, pixels.Color(0, 0, 0));
    }

    // Replace these LED configurations with the specific LED configuration for the Parietal Lobe
    // For the sake of this example, I'll use the same LED configurations as the Occipital Lobe
    
    for (int j = 20; j < 22; j++) {
      pixels.setPixelColor(j, colors[4]);
    }
    
    pixels.show();
    display.clearDisplay();

    display.setCursor(0, 0);
    display.setTextSize(2);
    display.println(texts[2]);
    display.display();
    delay(3000);
    display.clearDisplay();
    display.display();

    for (int j = 0; j < NUM_LEDS; j++) {
      pixels.setPixelColor(j, colors[0]);
    }
    pixels.show();
    server.send(200, "text/plain", "Occipital Lobe action executed");
  } else {
    server.send(200, "text/plain", "Animation is not running");
  }
}
void handleParietalLobe() {
  if (animationRunning) {
    // Insert logic for Parietal Lobe LED configuration here
    for (int j = 0; j < NUM_LEDS; j++) {
      pixels.setPixelColor(j, pixels.Color(0, 0, 0));
    }

    // Replace these LED configurations with the specific LED configuration for the Parietal Lobe
    // For the sake of this example, I'll use the same LED configurations as the Occipital Lobe
    
    for (int j = 23; j < 25; j++) {
      pixels.setPixelColor(j, colors[5]);
    }
    
    pixels.show();
    display.clearDisplay();

    display.setCursor(0, 0);
    display.setTextSize(2);
    display.println(texts[3]);
    display.display();
    delay(3000);
    display.clearDisplay();
    display.display();

    for (int j = 0; j < NUM_LEDS; j++) {
      pixels.setPixelColor(j, colors[0]);
    }
    pixels.show();
    server.send(200, "text/plain", "Parietal Lobe action executed");
  } else {
    server.send(200, "text/plain", "Animation is not running");
  }
}
void handleFrontalLobe() {
  if (animationRunning) {
    // Insert logic for Frontal Lobe LED configuration here
    for (int j = 0; j < NUM_LEDS; j++) {
      pixels.setPixelColor(j, pixels.Color(0, 0, 0));
    }

    // Replace these LED configurations with the specific LED configuration for the Parietal Lobe
    // For the sake of this example, I'll use the same LED configurations as the Occipital Lobe
    
    for (int j = 8; j < 12; j++) {
      pixels.setPixelColor(j, colors[6]);
    }
     for (int j = 21; j < 24; j++) {
      pixels.setPixelColor(j, colors[6]);
    }
    pixels.show();
    display.clearDisplay();

    display.setCursor(0, 0);
    display.setTextSize(2);
    display.println(texts[4]);
    display.display();
    delay(3000);
    display.clearDisplay();
    display.display();

    for (int j = 0; j < NUM_LEDS; j++) {
      pixels.setPixelColor(j, colors[0]);
    }
    pixels.show();
    server.send(200, "text/plain", "Frontal Lobe action executed");
  } else {
    server.send(200, "text/plain", "Animation is not running");
  }
}
void handleSensors() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  // Pulse sensor reading
  Signal = analogRead(PULSE_SENSOR_PIN);
  // Construct HTML response
  String html = "<html><body>";
  html += "<div style='display: inline-block; width: 100px; margin-right: 20px; border: 1px solid #000; padding: 10px;'>";
  html += "<h3>Temperature</h3>";
  html += "<p>" + String(temp) + " °C</p>";
  html += "</div>";
  html += "<div style='display: inline-block; width: 100px; border: 1px solid #000; padding: 10px;'>";
  html += "<h3>Humidity</h3>";
  html += "<p>" + String(hum) + " %</p>";
  html += "</div>";
  html += "<div style='display: inline-block; width: 100px; border: 1px solid #000; padding: 10px;'>";
  html += "<h3>Pulse Signal</h3>";
  html += "<p>" + String(Signal) + "</p>";
  html += "</div>";
  html += "</body></html>";

  // Send HTML response
  server.send(200, "text/html", html);
  
  // Display temperature and humidity on OLED screen
display.clearDisplay();
display.setTextSize(1);
display.setTextColor(SSD1306_WHITE);
display.setCursor(0, 0);
display.print("Temperature: ");
display.println(temp, 1);
display.print("Humidity: ");
display.println(hum, 1);
display.print("Pulse Signal: "); // New line to display pulse signal
display.println(Signal); // New line to display pulse signal value
display.display();

  
  // Change LED color based on temperature
  if (temp < 20) {
    pixels.fill(pixels.Color(0, 0, 255)); // Blue
  } else if (temp < 30) {
    pixels.fill(pixels.Color(255, 165, 0)); // Orange
  } else {
    pixels.fill(pixels.Color(255, 0, 0)); // Red
  }
  pixels.show();
  
  // LED behavior based on pulse sensor reading
  if (Signal > Threshold) {
    digitalWrite(LED_BUILTIN, HIGH); // Turn on LED if signal is above threshold
  } else {
    digitalWrite(LED_BUILTIN, LOW); // Turn off LED if signal is below threshold
  }
}
 // Determine which Signal to "count as a beat" and which to ignore.

void setup() {
  Serial.begin(115200);
  delay(100);
  pixels.begin();
  pixels.show(); // Initialize all pixels to 'off'
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
  display.clearDisplay();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  pixels.begin();
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.softAP(WIFI_SSID, WIFI_PSWD);
  IPAddress ip = WiFi.softAPIP();
  Serial.begin(9600);
  Serial.print("Access Point IP address: ");
  Serial.println(ip);
  pixels.show();  

  // Start the server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_GET, handleToggle);
  server.on("/temporallobe", HTTP_GET, handleTemporalLobe);
  server.on("/cerebellum", HTTP_GET, handleCerebellum);
  server.on("/occipitallobe", HTTP_GET, handleOccipitalLobe);
  server.on("/parietallobe", HTTP_GET, handleParietalLobe);
  server.on("/frontallobe", HTTP_GET, handleFrontalLobe);
  server.on("/sensors", HTTP_GET, handleSensors);
  server.begin();
  Serial.println("HTTP server started");
  dht.begin(); // Initialize DHT sensor
}

void loop() {
  server.handleClient();
}