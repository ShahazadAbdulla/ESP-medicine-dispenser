#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TFT_eSPI.h>  // Include TFT library
#include <ESP32Servo.h>

#define SERVO1_PIN 19   // Choose any GPIO
#define SERVO2_PIN 21   // Choose any GPIO
#define SWITCH1_PIN 27  // Pin for switch 1
#define SWITCH2_PIN 14  // Pin for switch 2
#define SWITCH3_PIN 25  // Pin for switch 3
#define SWITCH4_PIN 26  // Pin for switch 4
#define LED_BUZZ_PIN 32 // Led and Buzzer Pin

unsigned long previousMillis = 0;  // Store the last time the LED/buzzer was toggled
unsigned long interval = 500;       // Interval for beeping (in milliseconds)
bool ledState = false; 
bool alertActive = false;
int startSecond = 00;
bool servoState1 = false;
bool servoState2 = false;

Servo servo1, servo2;

TFT_eSPI tft = TFT_eSPI();  // Create TFT object

// 🌐 WiFi Credentials
#define WIFI_SSID "Wifi"
#define WIFI_PASS "password"

// 🌍 Time Setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);  // IST timezone (UTC +5:30)

// 💊 Medicine Reminder Variables
String medicine1 = "", medicine2 = "";
int startHour1 = -1, startMinute1 = -1, interval1 = 2;  // Default: 2-hour interval
int startHour2 = -1, startMinute2 = -1, interval2 = 4;  // Default: 4-hour interval
bool reminderTriggered1 = false, reminderTriggered2 = false;

// 🔗 Blynk Timer
BlynkTimer timer;

// 📌 Get Medicine 1 Name (V3)
BLYNK_WRITE(V3) {
  medicine1 = param.asStr();
  Serial.println("Medicine 1 set: " + medicine1);
  if (medicine1 == "") {
    Blynk.virtualWrite(V7, "No Reminder Set");
  }
  displayMedicineInfo();  // Call this function to display the updated medicine info
}

// 📌 Get Start Time for Medicine 1 (V4)
BLYNK_WRITE(V4) {
  TimeInputParam t(param);
  if (t.hasStartTime()) {
    startHour1 = t.getStartHour();
    startMinute1 = t.getStartMinute();
    reminderTriggered1 = false;
    Serial.printf("Medicine 1 Start Time: %02d:%02d\n", startHour1, startMinute1);
    Blynk.virtualWrite(V7, "Reminder Set: " + medicine1);
  } else {
    startHour1 = -1;
    startMinute1 = -1;
    Blynk.virtualWrite(V7, "No Reminder Set");
  }
  displayMedicineInfo();  // Call this function to display the updated medicine info
}

// 📌 Set Interval for Medicine 1 (V11)
BLYNK_WRITE(V11) {
  interval1 = param.asInt();
  Serial.printf("Medicine 1 Interval Set: Every %d Hours\n", interval1);
}

// 📌 Get Medicine 2 Name (V5)
BLYNK_WRITE(V5) {
  medicine2 = param.asStr();
  Serial.println("Medicine 2 set: " + medicine2);
  if (medicine2 == "") {
    Blynk.virtualWrite(V8, "No Reminder Set");
  }
  displayMedicineInfo();  // Call this function to display the updated medicine info
}

// 📌 Get Start Time for Medicine 2 (V6)
BLYNK_WRITE(V6) {
  TimeInputParam t(param);
  if (t.hasStartTime()) {
    startHour2 = t.getStartHour();
    startMinute2 = t.getStartMinute();
    reminderTriggered2 = false;
    Serial.printf("Medicine 2 Start Time: %02d:%02d\n", startHour2, startMinute2);
    Blynk.virtualWrite(V8, "Reminder Set: " + medicine2);
  } else {
    startHour2 = -1;
    startMinute2 = -1;
    Blynk.virtualWrite(V8, "No Reminder Set");
  }
  displayMedicineInfo();  // Call this function to display the updated medicine info
}

// 📌 Set Interval for Medicine 2 (V12)
BLYNK_WRITE(V12) {
  interval2 = param.asInt();
  Serial.printf("Medicine 2 Interval Set: Every %d Hours\n", interval2);
}

// 🕰 Check Reminder & Countdown
void checkReminder() {
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSeconds = timeClient.getSeconds();

  Serial.printf("Current Time: %02d:%02d:%02d\n", currentHour, currentMinute, currentSeconds);

  // Medicine 1 Reminder Logic
  if (medicine1 != "" && startHour1 != -1 && !reminderTriggered1) {
    int nextReminderHour1 = startHour1;
    while (nextReminderHour1 <= 23) {
      if (currentHour == nextReminderHour1 && currentMinute == startMinute1 && currentSeconds == startSecond) {
        // Show reminder message for Medicine 1
        Blynk.virtualWrite(V7, "⏰ Take " + medicine1 + "!");
        Serial.println("⚠ Medicine 1 Alert Triggered");
        reminderTriggered1 = true;
        alertPage(medicine1, currentHour, currentMinute);
        servoControl("servo1", 180);
        alertActive = true;
      }
      nextReminderHour1 += interval1;
    }
  }

  // Medicine 2 Reminder Logic
  if (medicine2 != "" && startHour2 != -1 && !reminderTriggered2) {
    int nextReminderHour2 = startHour2;
    while (nextReminderHour2 <= 23) {
      if (currentHour == nextReminderHour2 && currentMinute == startMinute2) {
        // Show reminder message for Medicine 2
        Blynk.virtualWrite(V8, "⏰ Take " + medicine2 + "!");
        Serial.println("⚠ Medicine 2 Alert Triggered");
        reminderTriggered2 = true;
        alertPage(medicine2, currentHour, currentMinute);
        // Move servo2 to 180°
        servoControl("servo2", 180);
        alertActive = true;
      }
      nextReminderHour2 += interval2;
    }
  }

  // Reset reminder message after 1 Minute (waiting for user confirmation)
  if (reminderTriggered1 && currentMinute != startMinute1) {
    // After a minute, reset the reminder message and show that it's waiting for confirmation
    Blynk.virtualWrite(V7, "Waiting for confirmation...");
    reminderTriggered1 = false;
  }
  if (reminderTriggered2 && currentMinute != startMinute2) {
    // After a minute, reset the reminder message and show that it's waiting for confirmation
    Blynk.virtualWrite(V8, "Waiting for confirmation...");
    reminderTriggered2 = false;
  }

  // Reset reminder message after confirmation
  if (!reminderTriggered1 && currentMinute != startMinute1) {
    Blynk.virtualWrite(V7, "Reminder Set: " + medicine1);
  }
  if (!reminderTriggered2 && currentMinute != startMinute2) {
    Blynk.virtualWrite(V8, "Reminder Set: " + medicine2);
  }

  // Auto Reset at Midnight
  if (currentHour == 0 && currentMinute == 0) {
    reminderTriggered1 = false;
    reminderTriggered2 = false;
    Blynk.virtualWrite(V7, "No Reminder Set");
    Blynk.virtualWrite(V8, "No Reminder Set");
  }
}

// Reset flags when switch is pressed to stop the alert and avoid repeated reminders
void resetAlert() {
  timeClient.update();

  // Get the current hour and minute
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();

  digitalWrite(LED_BUZZ_PIN, LOW);
  // Reset reminder flags
  reminderTriggered1 = false;
  reminderTriggered2 = false;
  alertActive = false;

  // Reset TFT screen to show that the alert is reset
  frontPage();  // You can modify this function to reset the display as needed
  displayMedicineInfo();  // Update with the current status of the medicines

  
  tft.fillRect(0, 120, 128, 40, TFT_BLACK);  // Clear bottom section for time

  // Set text size and color for time display
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(65, 140);  // Set position for time display

  // Print the current time in HH:MM format
  tft.print(currentHour < 10 ? "0" : "");  // Leading zero for single-digit hours
  tft.print(currentHour);
  tft.print(":");
  tft.print(currentMinute < 10 ? "0" : "");  // Leading zero for single-digit minutes
  tft.println(currentMinute);


  Serial.println("Alert reset");
}


// Function to control servos
void servoControl(String servoName, int angle) {
  Serial.println("enetered servo");
  if (servoName == "servo1") {
    servo1.attach(SERVO1_PIN);
    Serial.println("moving servo1");
    servo1.write(angle);  // Set the angle for servo 1
    delay(1500);
  } else if (servoName == "servo2") {
    Serial.println("moving servo2");
    servo2.attach(SERVO2_PIN);
    servo2.write(angle);  // Set the angle for servo 2
    delay(1500);
  }
}

int prevHour = -1;   // Store the previous hour
int prevMinute = -1; // Store the previous minute

void displayTime() {
  // Update the time from the NTP client
  timeClient.update();

  // Get the current hour and minute
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  
  // Check if the current hour or minute has changed
  if (currentHour != prevHour || currentMinute != prevMinute) {
    // If yes, update the time display
    tft.fillRect(0, 120, 128, 40, TFT_BLACK);  // Clear bottom section for time

    // Set text size and color for time display
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(65, 140);  // Set position for time display

    // Print the current time in HH:MM format
    tft.print(currentHour < 10 ? "0" : "");  // Leading zero for single-digit hours
    tft.print(currentHour);
    tft.print(":");
    tft.print(currentMinute < 10 ? "0" : "");  // Leading zero for single-digit minutes
    tft.println(currentMinute);

    // Store the current hour and minute as previous values for the next loop
    prevHour = currentHour;
    prevMinute = currentMinute;
  }
}

void alertPage(String medicine, int hour, int min){
  tft.fillRect(0, 0, 128, 160, 0x00);
  tft.drawRect(0, 0, 128, 55, 0x57FF);
  tft.setTextColor(0x57EA);
  tft.setTextSize(2);
  tft.setCursor(40, 10);
  tft.println("TAKE");
  tft.setCursor(10, 30);
  tft.println("MEDICINE!");
  tft.setCursor(30, 70);
  tft.setTextSize(1.5);
  tft.println(medicine);
  // Set text size and color for time display
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(65, 140);  // Set position for time display

  // Print the current time in HH:MM format
  tft.print(hour < 10 ? "0" : "");  // Leading zero for single-digit hours
  tft.print(hour);
  tft.print(":");
  tft.print(min < 10 ? "0" : "");  // Leading zero for single-digit minutes
  tft.println(min);
}

void displayMedicineInfo() {
  // Clear previous medicine info
  tft.fillRect(0, 80, 128, 40, TFT_BLACK);  // Clear area where medicine info will be displayed

  // Set text size and color
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  
  // Display medicine 1 name and start time
  if (medicine1 != "") {
    tft.setCursor(10, 80);  // Set cursor for first medicine
    tft.print(medicine1);
    tft.print(" @ ");
    tft.print(startHour1 < 10 ? "0" : "");  // Leading zero for single-digit hour
    tft.print(startHour1);
    tft.print(":");
    tft.print(startMinute1 < 10 ? "0" : "");  // Leading zero for single-digit minute
    tft.println(startMinute1);
  }

  // Display medicine 2 name and start time
  if (medicine2 != "") {
    tft.setCursor(10, 90);  // Set cursor for second medicine
    tft.print(medicine2);
    tft.print(" @ ");
    tft.print(startHour2 < 10 ? "0" : "");  // Leading zero for single-digit hour
    tft.print(startHour2);
    tft.print(":");
    tft.print(startMinute2 < 10 ? "0" : "");  // Leading zero for single-digit minute
    tft.println(startMinute2);
  }
}

void frontPage(){
    tft.fillRect(0, 0, 128, 160, 0x0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("MEDICINE");
    tft.setCursor(10, 30);
    tft.println("DISPENSER");
    tft.setTextSize(1.5);
    tft.setCursor(10, 70);
    tft.println("REMINDERS:");
}


void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  pinMode(SWITCH1_PIN, INPUT_PULLUP);
  pinMode(SWITCH2_PIN, INPUT_PULLUP);
  pinMode(SWITCH3_PIN, INPUT_PULLUP);
  pinMode(SWITCH4_PIN, INPUT_PULLUP);
  pinMode(LED_BUZZ_PIN, OUTPUT);

  tft.begin();
  tft.setRotation(4);  // Adjust rotation if necessary

  frontPage();

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  timeClient.begin();

  Serial.println("Blynk and ntp begin");

  // Run checkReminder() every second for live countdown
  timer.setInterval(1000L, checkReminder);
}

void loop() {
  Blynk.run();
  timer.run();

  // Display the updated time every second
  displayTime();

  if (alertActive) {
   unsigned long currentMillis = millis();

    // If the interval has passed, toggle the LED/buzzer state
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;  // Save the current time
      ledState = !ledState;             // Toggle the state of LED/buzzer
      digitalWrite(LED_BUZZ_PIN, ledState ? HIGH : LOW);  // Set the new state
    }
  }

  // ✅ Wait for switch1 press to reset servos with consumption
  if (digitalRead(SWITCH1_PIN) == LOW) {

    tft.fillRect(0, 0, 128, 160, 0x0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(20, 40);
    tft.println("MEDICINE");
    tft.setCursor(20, 60);
    tft.println("CONSUMED");

    delay(3000);

    servoControl("servo1", 0);
    servoControl("servo2", 0);
    if (reminderTriggered1){
      medicine1 = "";
      Serial.println("medicine 1 cleared");
    }
    if (reminderTriggered2){
      medicine2 = "";
      Serial.println("medicine 2 cleared");
    }

    resetAlert();

    Serial.println("Medicine Taken - Resetting Servos");
  }

  // ✅ Wait for switch2 press to reset servos without consumption
  if (digitalRead(SWITCH2_PIN) == LOW) {

    tft.fillRect(0, 0, 128, 160, 0x0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(20, 40);
    tft.println("MEDICINE");
    tft.setCursor(45, 60);
    tft.println("NOT");
    tft.setCursor(20, 80);
    tft.println("CONSUMED");

    delay(3000);

    Blynk.logEvent("consumption");

    servoControl("servo1", 0);
    servoControl("servo2", 0);
    resetAlert();

    Serial.println("Medicine Taken - Resetting Servos");
  }
  //for servo1 refilling
  if (digitalRead(SWITCH3_PIN) == LOW){
    if (servoState1){
      servoControl("servo1", 0);
      servoState1 = !servoState1;
    }
    else{
      servoControl("servo1", 180);
      servoState1 = !servoState1;
    }
  }
  //for servo2 refilling
  if (digitalRead(SWITCH4_PIN) == LOW){
    if (servoState2){
      servoControl("servo2", 0);
      servoState2 = !servoState2;
    }
    else{
      servoControl("servo2", 180);
      servoState2 = !servoState2;
    }
  }
}