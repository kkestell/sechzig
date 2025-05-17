#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include "time.h"

#define LED_PIN 4    // NeoPixel data pin
#define NUM_PIXELS 60 // Number of NeoPixels in your ring

// NTP Server config
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -21600;  // -6 hours for CST (-6 * 3600 seconds)
const int daylightOffset_sec = 0;   // Set to 3600 during DST if needed

// WiFi credentials
const char* ssid = "K";
const char* password = "mihzyn-vomcYc-2nejwy";

Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

bool timeInitialized = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting NeoPixel Clock");
  
  // Initialize the NeoPixel strip
  pixels.begin();
  pixels.setBrightness(50);  // Set brightness to 50 (out of 255)
  pixels.clear();
  
  // Make all pixels green to show we're initializing
  for(int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 50, 0));
  }
  pixels.show();
  
  // Connect to WiFi for time sync
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    
    // Make all pixels blue to show we're syncing time
    for(int i = 0; i < NUM_PIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 50));
    }
    pixels.show();
    
    // Init and get the time with CST timezone
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    // Wait for time to be set
    struct tm timeinfo;
    int timeAttempts = 0;
    while(!getLocalTime(&timeinfo) && timeAttempts < 10) {
      Serial.println("Waiting for time sync...");
      delay(1000);
      timeAttempts++;
    }
    
    if(getLocalTime(&timeinfo)) {
      Serial.printf("Time synchronized: %02d:%02d:%02d\n", 
                   timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
      timeInitialized = true;
      
      // Disconnecting WiFi as it's no longer needed
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      Serial.println("WiFi disconnected");
    } else {
      Serial.println("Failed to obtain time from NTP, keeping WiFi on");
    }
  } else {
    Serial.println("\nWiFi connection failed");
  }
  
  // Clear pixels again
  pixels.clear();
  pixels.show();
}

void loop() {
  struct tm timeinfo;
  
  // Get current time
  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    
    // Red flash to indicate error
    for(int i = 0; i < NUM_PIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(50, 0, 0));
    }
    pixels.show();
    delay(500);
    pixels.clear();
    pixels.show();
    delay(500);
    return;
  }
  
  Serial.printf("Time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  
  // Clear all pixels
  pixels.clear();
  
  // Calculate positions (0 = top = 12 o'clock position)
  // For a 60-pixel ring, minutes and seconds map directly (0-59)
  // Hours need to map from 12-hour format to position on the ring
  int hourPosition = (timeinfo.tm_hour % 12) * 5 + timeinfo.tm_min / 12;
  int minutePosition = timeinfo.tm_min;
  int secondPosition = timeinfo.tm_sec;
  
  // Set hour pixel (red)
  pixels.setPixelColor(hourPosition, pixels.Color(255, 0, 0));
  
  // Set minute pixel (blue)
  pixels.setPixelColor(minutePosition, pixels.Color(0, 0, 255));
  
  // Set second pixel (green)
  pixels.setPixelColor(secondPosition, pixels.Color(0, 255, 0));
  
  // Show the pixels
  pixels.show();
  
  // Update once per second
  delay(1000);
}