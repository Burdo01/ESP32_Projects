#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "";
const char* password = "";

// CoinGecko API endpoint for Dogecoin price in USD
const char* apiUrl = "https://api.coingecko.com/api/v3/simple/price?ids=dogecoin&vs_currencies=usd";

// Define the pin numbers
const int secondLightPin = 2; // Second light connected to GPIO 2

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Initialize esp32 light as outputs
  pinMode(secondLightPin, OUTPUT);
  

  displaySplash(); // splash boot screen
  connectWifi(); // set up wifi and display info 
  displayMessage(); // ready device and project title

}

void loop() {
  // put your main code here, to run repeatedly:
  // Fetch the Dogecoin price
  String price = getDogecoinPrice();

  // Clear the display before printing the new value
  display.clearDisplay();

  displayPrice(price); // pass the price and print on display

  fifteenseccount(price); // sloppy code for countdown

}

void displaySplash() {
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(1000); // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();
}

void connectWifi() {
  // Connecting to Wi-Fi
  Serial.println("Connecting to WiFi...");
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);
  display.print("Connecting to WiFi...");
  display.display();

  WiFi.mode(WIFI_STA); // Optional
  WiFi.begin(ssid, password);

  // Wait until connected to Wi-Fi
  unsigned long startAttemptTime = millis(); // Start a timer to avoid hanging forever
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startAttemptTime >= 15000) { // Timeout after 15 seconds
      Serial.println("Failed to connect to WiFi.");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Failed to connect to WiFi.");
      display.display();
      return;
    }
    
    Serial.print(".");
    display.print(".");
    display.display();
    delay(500);
  }

  // Clear the buffer
  display.clearDisplay();

  // Turn on the second light
  digitalWrite(secondLightPin, HIGH);

  // Print the IP address once connected
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);
  display.print("WiFi connected!");
  display.setCursor(0,15);
  display.print("IP Address: ");
  display.print(WiFi.localIP());
  display.display();
  delay(2000);

  // Clear the buffer
  display.clearDisplay();
}

void displayMessage(){

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);
  display.print("Ready to display...");

  display.setTextSize(2);             // Double 2:1 pixel scale
  display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
  display.setCursor(20,25);
  display.println("Crypto");
  display.setCursor(20, 45);
  display.println("Prices");

  display.display();
  delay(3000);

  //Clear the buffer
  display.clearDisplay();
}

// Function to fetch Dogecoin price from CoinGecko API
String getDogecoinPrice() {
  HTTPClient http;
  http.begin(apiUrl); // Start HTTP request

  int httpCode = http.GET(); // Make GET request
  String payload = "{}"; // Initialize payload

  if (httpCode == 200) { // If HTTP request is successful
    payload = http.getString();
  }
  http.end(); // Close the connection

  // Parse JSON response
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  float dogePrice = doc["dogecoin"]["usd"]; // Get the price from JSON response

  return String(dogePrice, 6); // Return price as string with 5 decimal places
}

void displayPrice(String p){
  // Display Dogecoin price
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Dogecoin Price (USD):");
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print("$" + p); // Print the fetched Dogecoin price
  display.display();
}

void fifteenseccount(String p){
  delay(5000);
  display.setCursor(0, 50);
  display.setTextSize(1);
  display.print("Updating... 10");
  display.display();
  delay(1000);
  display.clearDisplay();

  displayPrice(p);
  display.setCursor(0, 50);
  display.setTextSize(1);
  display.print("Updating... 9");
  display.display();
  delay(1000);
  display.clearDisplay();

  displayPrice(p);
  display.setCursor(0, 50);
  display.setTextSize(1);
  display.print("Updating... 8");
  display.display();
  delay(1000);
  display.clearDisplay();

  displayPrice(p);
  display.setCursor(0, 50);
  display.setTextSize(1);
  display.print("Updating... 7");
  display.display();
  delay(1000);
  display.clearDisplay();

  displayPrice(p);
  display.setCursor(0, 50);
  display.setTextSize(1);
  display.print("Updating... 6");
  display.display();
  delay(1000);
  display.clearDisplay();

  displayPrice(p);
  display.setCursor(0, 50);
  display.setTextSize(1);
  display.print("Updating... 5");
  display.display();
  delay(1000);
  display.clearDisplay();

  displayPrice(p);
  display.setCursor(0, 50);
  display.setTextSize(1);
  display.print("Updating... 4");
  display.display();
  delay(1000);
  display.clearDisplay();

  displayPrice(p);
  display.setCursor(0, 50);
  display.setTextSize(1);
  display.print("Updating... 3");
  display.display();
  delay(1000);
  display.clearDisplay();

  displayPrice(p);
  display.setCursor(0, 50);
  display.setTextSize(1);
  display.print("Updating... 2");
  display.display();
  delay(1000);
  display.clearDisplay();

  displayPrice(p);
  display.setCursor(0, 50);
  display.setTextSize(1);
  display.print("Updating... 1");
  display.display();
  delay(1000);
}