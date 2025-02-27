// #include "sdkconfig.h"
// #if CONFIG_ESP_WIFI_REMOTE_ENABLED
// #error "WiFiProv is only supported in SoCs with native Wi-Fi support"
// #endif

// #include "WiFiProv.h"
// #include<gpio_viewer.h>
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
// #include <Base64.h>
// #include <time.h>  // for time() function
// #include <tweetnacl.h>
// #include <EEPROM.h>
// EEPROM.write(NUMBER_ADDR, piShares);
//   // Read the number from EEPROM on startup
//   piShares = EEPROM.read(NUMBER_ADDR);
  
// const double pLo = 0; // EEPROM address to store the number
// const double pHi = 1;
// const double dLo = 2;
// const double dHi = 3;

double piHigh = 0;
double piLow = 0;
double dogeHigh = 0;
double dogeLow = 0;

// Replace with your network credentials
const char* ssid = "SSD";
const char* password = "PASS";

// GPIOViewer gpio_viewer;

// CoinGecko API endpoint for Dogecoin price in USD
const char* dogeAPI = "https://api.coingecko.com/api/v3/simple/price?ids=dogecoin&vs_currencies=usd";
// Pi cryptocurrency API URL (replace with actual source of Pi price)
const char* piAPI = "https://api.coingecko.com/api/v3/simple/price?ids=pi-network&vs_currencies=usd";
const char* dogeLabel = "DogeCoin Price (USD):";
const char* piLabel = "Pi Network Price :";
const double dogeShares = 0;
double piShares = 0;

// Define the pin numbers
const int secondLightPin = 2; // Second light connected to GPIO 2

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// // code for updating wifi credentials w/o code
// // #define USE_SOFT_AP // Uncomment if you want to enforce using the Soft AP method instead of BLE (wifi or bluetooth)
// const char *pop = "abcd1234";           // Proof of possession - otherwise called a PIN - string provided by the device, entered by the user in the phone app
// const char *service_name = "PROV_123";  // Name of your device (the Espressif apps expects by default device name starting with "Prov_")
// const char *service_key = NULL;         // Password used for SofAP method (NULL = no password needed)
// bool reset_provisioned = false;          // When true the library will automatically delete previously provisioned data.


void setup() { // put your setup code here, to run once:
  
  Serial.begin(115200);

  pinMode(secondLightPin, OUTPUT); // Initialize esp32 light as output

  // gpio_viewer.connectToWifi("SSID", "PASS"); // if not using wifi in project

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C I2C connection to OLED display
    Serial.println(F("SSD1306 allocation failed")); // screen connection check
    for(;;); // Don't proceed, loop forever
  }

  displaySplash(); // splash boot screen
  connectWifi(); // set up wifi and display info 
  displayMessage(); // ready device and project title

  // gpio_viewer.connectToWifi("SSID", "PASS"); // if not using wifi in project
  // gpio_viewer.begin();

  // wifiProvSetup();
}

void loop() { // put your main code here, to run repeatedly:
  
  String dogePrice = getDogecoinPrice(); // Fetch the Dogecoin price
  String piPrice = getPiPrice();

  loopPrice(dogePrice, dogeLabel, dogeShares, dogeHigh, dogeLow); // pass price and name to loop with animation
  loopPrice(piPrice, piLabel, piShares, piHigh, piLow);
  addPi();

}

void connectWifi(void) {
  // Connecting to Wi-Fi
  Serial.println("Connecting to WiFi...");
  resetDisplayText();
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
    displayDelay(500);
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

  resetDisplayText();
  display.print("WiFi connected!");
  display.setCursor(0,15);
  display.print("IP Address: ");
  display.print(WiFi.localIP());
  displayDelay(1000);

  // Clear the buffer
  display.clearDisplay();
  display.clearDisplay();
}

void displayMessage(void){

  resetDisplayText();
  display.print("Ready to display...");

  display.setTextSize(2);             // Double 2:1 pixel scale
  display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
  display.setCursor(18,25);
  display.println(" Crypto ");
  display.setCursor(18, 45);
  display.println(" Prices ");

  displayDelay(1500);

  //Clear the buffer
  display.clearDisplay();
}

// Function to fetch Dogecoin price from CoinGecko API
String getDogecoinPrice(void) {

  HTTPClient http;
  http.begin(dogeAPI); // Start HTTP request

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

  return String(dogePrice, 6); // Return price as string with 6 decimal places
}

String getPiPrice(void) {
  HTTPClient http;
  http.begin(piAPI);
  int httpCode = http.GET();

  if (httpCode > 0) { // Check if the HTTP request was successful
    String payload = http.getString();
    Serial.println(payload);

    // Parse the JSON response
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    float piPrice = doc["pi-network"]["usd"]; // Extract the USD price

    // Return the price as a string
    return String(piPrice, 6); // Limit the number of decimal places
  } else {
    Serial.println("Error in HTTP request");
    return "N/A";
  }
}

void displayPrice(String p, String n){ // Display crypto price
  
  resetDisplayText();
  display.print(n); // print name/label of crypto
  display.setTextSize(2);
  display.setCursor(0, 15);
  display.print("$" + p); // Print the fetched Crypto price
  display.display();
}

void fifteenSecCount(String p, String n, double s, double hi, double lo){

  delay(5000);
  display.setCursor(100, 55);
  display.setTextSize(1);
  // display.print("10"); // countdown
  displayDelay(1000);
  display.clearDisplay();


  for(int i = 9; i > 0; i--){
    updateScreen(p, n, s, hi, lo);
    String val = String(i);
    // display.print(val); // countdown
    displayDelay(1000);
    display.clearDisplay();
  }
}

void displayShares(double s){
  display.setCursor(0, 35);
  display.setTextSize(1);
  String val = String(s);
  display.print("Shares: " + val);
  display.display();
}

void calcShareValue(String p, double s){
  display.setCursor(0, 45);
  display.setTextSize(1);
  double price = p.toDouble();
  double currVal = s * price;
  String val = String(currVal);
  //sprintf(buffer, "%.2f", currVal); // Formats to 2 decimal places
  display.print("Value: $" + val);
  display.display();
}

void updateScreen(String p, String n, double s, double hi, double lo){
  displayPrice(p, n);
  displayShares(s);
  calcShareValue(p, s);
  displayHighLow(hi, lo);
  display.setCursor(100, 55);
  display.setTextSize(1);
}

void displayDelay(int d){
  display.display();
  delay(d);
}

void resetDisplayText(void){
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);
}

void loopPrice(String p, String n, double s, double hi, double lo){

  displayPrice(p, n); // pass the price and print on display
  displayShares(s);
  calcShareValue(p, s);
  trackHighLow(p, n);
  displayHighLow(hi, lo);
  fifteenSecCount(p, n, s, hi, lo); // sloppy code for countdown
  drawLineAnimation();
}

void addPi(){
  piShares += .00008;  // per 30 sec (15sec loops)
}

void trackHighLow(String p, String n){

  double check = p.toDouble();
  //float check = p.toFloat();

  if (n == piLabel) {
    if (piHigh < check) {
        piHigh = check;
    }
    if (piLow > check || piLow == 0) {
        piLow = check;
    }
  }

  if (n == dogeLabel) {
    if (dogeHigh < check) {
        dogeHigh = check;
    }
    if (dogeLow > check || dogeLow == 0) {
        dogeLow = check;
    }
  }
}

void displayHighLow(double hi, double lo){
  display.setCursor(0, 55);
  display.setTextSize(1);
  String strHi = String(hi); 
  String strLo = String(lo); 
  display.print("L$");
  display.print(lo, 6);
  display.print(" H$");
  display.print(hi, 6);
  display.display();
}

// animations at startup and between updates
void displaySplash(void) {
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  displayDelay(500);
  display.clearDisplay(); // Clear the buffer
}

void drawLineAnimation() {
  int16_t i;

  display.clearDisplay();

  for(i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, WHITE);
    display.display();
    //delay(1);
  }
  for(i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, WHITE);
    display.display();
    //delay(1);
  }
  display.clearDisplay();
}

void testfillcircle(void) {
  display.clearDisplay();

  for(int16_t i=max(display.width(),display.height())/2; i>0; i-=3) {
    // The INVERSE color is used so circles alternate white/black
    display.fillCircle(display.width() / 2, display.height() / 2, i, INVERSE);
    display.display(); // Update screen with each newly-drawn circle
    delay(1);
  }

  delay(2000);
  display.clearDisplay(); // Clear the buffer
}

void testdrawrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.display(); // Update screen with each newly-drawn rectangle
    delay(1);
  }

  delay(2000);
  display.clearDisplay(); // Clear the buffer
}

void testscrolltext(void) {
  display.clearDisplay();

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.println(F("scroll"));
  display.display();      // Show initial text
  delay(100);

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
  delay(1000);
}

// // Event to change wifi provisions
// // WARNING: SysProvEvent is called from a separate FreeRTOS task (thread)!
// void SysProvEvent(arduino_event_t *sys_event) {
//   switch (sys_event->event_id) {
//     case ARDUINO_EVENT_WIFI_STA_GOT_IP:
//       Serial.print("\nConnected IP address : ");
//       Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
//       break;
//     case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: Serial.println("\nDisconnected. Connecting to the AP again... "); break;
//     case ARDUINO_EVENT_PROV_START:            Serial.println("\nProvisioning started\nGive Credentials of your access point using smartphone app"); break;
//     case ARDUINO_EVENT_PROV_CRED_RECV:
//     {
//       Serial.println("\nReceived Wi-Fi credentials");
//       Serial.print("\tSSID : ");
//       Serial.println((const char *)sys_event->event_info.prov_cred_recv.ssid);
//       Serial.print("\tPassword : ");
//       Serial.println((char const *)sys_event->event_info.prov_cred_recv.password);
//       break;
//     }
//     case ARDUINO_EVENT_PROV_CRED_FAIL:
//     {
//       Serial.println("\nProvisioning failed!\nPlease reset to factory and retry provisioning\n");
//       if (sys_event->event_info.prov_fail_reason == NETWORK_PROV_WIFI_STA_AUTH_ERROR) {
//         Serial.println("\nWi-Fi AP password incorrect");
//       } else {
//         Serial.println("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");
//       }
//       break;
//     }
//     case ARDUINO_EVENT_PROV_CRED_SUCCESS: Serial.println("\nProvisioning Successful"); break;
//     case ARDUINO_EVENT_PROV_END:          Serial.println("\nProvisioning Ends"); break;
//     default:                              break;
//   }
// }

// void wifiProvSetup(){
//     // backup wifi provision to change network
//   WiFi.begin();  // no SSID/PWD - get it from the Provisioning APP or from NVS (last successful connection)
//   WiFi.onEvent(SysProvEvent);

//   // BLE Provisioning using the ESP SoftAP Prov works fine for any BLE SoC, including ESP32, ESP32S3 and ESP32C3.
//   #if CONFIG_BLUEDROID_ENABLED && !defined(USE_SOFT_AP)
//     Serial.println("Begin Provisioning using BLE");
//     // Sample uuid that user can pass during provisioning using BLE
//     uint8_t uuid[16] = {0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf, 0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02};
//     WiFiProv.beginProvision(
//       NETWORK_PROV_SCHEME_BLE, NETWORK_PROV_SCHEME_HANDLER_FREE_BLE, NETWORK_PROV_SECURITY_1, pop, service_name, service_key, uuid, reset_provisioned
//     );
//     log_d("ble qr");
//     WiFiProv.printQR(service_name, pop, "ble");
//   #else
//     Serial.println("Begin Provisioning using Soft AP");
//     WiFiProv.beginProvision(NETWORK_PROV_SCHEME_SOFTAP, NETWORK_PROV_SCHEME_HANDLER_NONE, NETWORK_PROV_SECURITY_1, pop, service_name, service_key);
//     log_d("wifi qr");
//     WiFiProv.printQR(service_name, pop, "softap");
//   #endif
// }
