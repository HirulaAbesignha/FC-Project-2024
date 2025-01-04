#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include "esp_sleep.h"  // For deep sleep functionality

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

// Pin definitions
const int soilMoisturePin = 36;  // Analog pin for the soil moisture sensor
const int motorPin = 16;         // GPIO16 (RX2) for motor pump control
const int airValue = 4095;       // Sensor value when soil is dry
const int waterValue = 0;        // Sensor value when soil is fully wet
const int moistureThreshold = 10; // Threshold percentage for soil moisture (10% as per request)

const int uvSensorPin = 39;      // Analog pin for UV sensor (GUVA S12SD)
const int buttonPin = 4;         // Pin for button to display sunlight level
const int waterPumpButtonPin = 12; // Pin for button to manually control the water pump
const int moistureButtonPin = 32;  // Pin for button to display moisture level
const int waterLevelPin = 34;    // Pin for water level sensor (analog)

const int sunlightLedPin = 13;   // Pin for the sunlight LED bulb
const int motorLedPin = 14;      // Pin for the motor LED bulb
const int moistureLedPin = 25;   // Pin for the moisture LED (indicates low moisture level)
const int powerLedPin = 27;      // Pin for power status LED

// Power Button Pin
const int powerButtonPin = 33;   // Pin for power button

unsigned long previousMillis = 0;  // Store the time for alternating screens
const long interval = 2000;         // Interval to alternate screens (2 seconds)
bool isPowerOn = true; // Flag to track whether the ESP32 is on or off

void setup() {
  lcd.init();              // Initialize the LCD
  lcd.backlight();         // Turn on the backlight
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  Serial.begin(115200);    // Initialize serial communication for debugging
  Wire.begin();

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  pinMode(motorPin, OUTPUT);   // Set motor pin as output
  digitalWrite(motorPin, LOW); // Ensure motor is off initially

  pinMode(buttonPin, INPUT_PULLUP); // Configure button pin with internal pull-up
  pinMode(waterPumpButtonPin, INPUT_PULLUP); // Configure water pump button
  pinMode(sunlightLedPin, OUTPUT); // Set sunlight LED pin as output
  pinMode(motorLedPin, OUTPUT);    // Set motor LED pin as output
  pinMode(moistureLedPin, OUTPUT); // Set moisture LED pin as output
  pinMode(powerLedPin, OUTPUT);    // Set power LED pin as output
  pinMode(powerButtonPin, INPUT_PULLUP); // Set power button pin with pull-up resistor
  pinMode(moistureButtonPin, INPUT_PULLUP); // Set moisture button pin
  pinMode(waterLevelPin, INPUT);    // Pin for water level sensor

  delay(1000);  // Allow time for initialization message
  lcd.clear();
  
  // Display power status
  if (isPowerOn) {
    digitalWrite(powerLedPin, HIGH);  // Turn on power LED when ESP32 is on
  } else {
    digitalWrite(powerLedPin, LOW);   // Turn off power LED when ESP32 is off
  }
}

// The loop function continuously executes and manages tasks like reading sensors, updating displays, and responding to button presses.
void loop() {
  if (digitalRead(powerButtonPin) == LOW) {  // Check if power button is pressed
    delay(200);  // Debounce delay

    // Toggle power state
    if (isPowerOn) {
      goToSleep();  // Put ESP32 in deep sleep (turn off)
    } else {
      wakeUp();  // Wake up from deep sleep (turn on)
    }
  }

  if (isPowerOn) {
    // Only execute the following tasks if the ESP32 is powered on
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      displayDateTime();
    }

    checkSunlightLevel();
    handleButtonPresses();
    checkScheduledWatering();  // Check for scheduled watering based on time and moisture level
    checkWaterLevel();         // Check water level
  }
}

// Add the function to check if the time is within a certain range
bool isTimeInRange(DateTime now) { 
    return ((now.hour() == 6 && now.minute() >= 30) || 
            (now.hour() == 7) || 
            (now.hour() == 8 && now.minute() <= 30) || 
            (now.hour() == 14 && now.minute() >= 30) || 
            (now.hour() == 15) || 
            (now.hour() == 16 && now.minute() <= 30)); 
}

// Function to check if it's time for scheduled watering
void checkScheduledWatering() {
  DateTime now = rtc.now();  // Get the current time from RTC
  int soilMoistureLevel = analogRead(soilMoisturePin); // Read the soil moisture level
  int moisturePercentage = map(soilMoistureLevel, waterValue, airValue, 0, 100); // Map the moisture to percentage

  // Check if the current time is within the scheduled watering times
  if (isTimeInRange(now) && moisturePercentage < moistureThreshold) {
    controlWaterPump(15000);  // Turn on the pump for 15 seconds if moisture is below threshold
  }
}

void controlWaterPump(unsigned long duration) {
  digitalWrite(motorPin, HIGH); // Turn on the motor
  digitalWrite(motorLedPin, HIGH); // Turn on the motor LED
  Serial.println("Watering started for 15 seconds...");
  
  delay(duration); // Wait for the specified duration

  digitalWrite(motorPin, LOW); // Turn off the motor after the duration
  digitalWrite(motorLedPin, LOW); // Turn off the motor LED
  Serial.println("Watering Done.");
}

void displayDateTime() {
  DateTime now = rtc.now();

  // Always display time and date on the LCD
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  printTwoDigits(now.hour());
  lcd.print(":");
  printTwoDigits(now.minute());
  lcd.print(":");
  printTwoDigits(now.second());

  lcd.setCursor(0, 1);
  lcd.print("Date: ");
  lcd.print(now.year());
  lcd.print("/");
  printTwoDigits(now.month());
  lcd.print("/");
  printTwoDigits(now.day());
  Serial.print("Date: ");
  Serial.print(now.year());
  Serial.print("/");
  printTwoDigits(now.month());
  Serial.print("/");
  printTwoDigits(now.day());
  Serial.print("  Time: ");
  printTwoDigits(now.hour());
  Serial.print(":");
  printTwoDigits(now.minute());
  Serial.print(":");
  printTwoDigits(now.second());
  Serial.println();
}

void checkSunlightLevel() {
  int uvValue = analogRead(uvSensorPin);
  int sunlightPercent = map(uvValue, 0, 4095, 0, 100);
  sunlightPercent = constrain(sunlightPercent, 0, 100);

  if (sunlightPercent < 10) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sunlight Low:");
    lcd.setCursor(0, 1);
    lcd.print(sunlightPercent);
    lcd.print("%");
    Serial.print("Sunlight Low: ");
    Serial.print(sunlightPercent);
    Serial.println("%");
    digitalWrite(sunlightLedPin, HIGH); // Turn on the sunlight LED
  } else {
    digitalWrite(sunlightLedPin, LOW); // Turn off the sunlight LED
  }
}

void checkWaterLevel() {
  int waterLevel = analogRead(waterLevelPin);  // Read the water level sensor
  int waterLevelPercentage = map(waterLevel, 0, 4095, 0, 100);  // Convert to percentage

  // If water level is below 15%, turn on LED and display on LCD
  if (waterLevelPercentage < 15) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Water Level Low:");
    lcd.setCursor(0, 1);
    lcd.print(waterLevelPercentage);
    lcd.print("%");
    Serial.print("Water level low: ");
    Serial.print(waterLevelPercentage);
    Serial.println("%");
    digitalWrite(sunlightLedPin, HIGH);  // Turn on the LED indicating low water level
  } else {
    digitalWrite(sunlightLedPin, LOW);  // Turn off the LED when water level is above 15%
  }
}

void handleButtonPresses() {
  if (digitalRead(buttonPin) == LOW) {  // Button to display sunlight level
    displaySunlightLevel();
  }

  if (digitalRead(moistureButtonPin) == LOW) {  // Button to display moisture level
    displaySoilMoisture();
  }

  if (digitalRead(waterPumpButtonPin) == LOW) {  // Button to manually control water pump
    controlWaterPump(15000);  // Start watering
  }
}

void displaySunlightLevel() {
  int uvValue = analogRead(uvSensorPin);
  int sunlightPercent = map(uvValue, 0, 4095, 0, 100);
  sunlightPercent = constrain(sunlightPercent, 0, 100);

  Serial.print("Sunlight Level: ");
  Serial.print(sunlightPercent);
  Serial.println("%");
}

void displaySoilMoisture() {
  int sensorValue = analogRead(soilMoisturePin); // Read the sensor value
  int moisturePercent = map(sensorValue, airValue, waterValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100); // Ensure the value is between 0 and 100

  Serial.print("Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  // Turn on the moisture LED if the moisture is below 10%
  if (moisturePercent < moistureThreshold) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Moisture Low:");
    lcd.setCursor(0, 1);
    lcd.print(moisturePercent);
    lcd.print("%");
    digitalWrite(moistureLedPin, HIGH);  // Turn on moisture LED
  } else {
    digitalWrite(moistureLedPin, LOW);   // Turn off moisture LED
  }
}

void printTwoDigits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}
