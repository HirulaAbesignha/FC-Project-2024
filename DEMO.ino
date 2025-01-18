
#include <Wire.h>  // Includes the Wire library for I2C communication.
#include <LiquidCrystal_I2C.h>  // Includes the LiquidCrystal_I2C library for controlling an I2C-based LCD display.
#include <ThreeWire.h>  // Includes the ThreeWire library to communicate with the DS1302 RTC using three wires.
#include <RtcDS1302.h>  // Includes the RtcDS1302 library for interfacing with the DS1302 Real-Time Clock.
#include <RTClib.h>

//PIN DEFINING
#define IO 15    // Defines the data (DAT) pin for DS1302 RTC.
#define SCLK 18  // Defines the clock (CLK) pin for DS1302 RTC.
#define CE 2    // Defines the reset (RST) pin for DS1302 RTC.
#define waterSensorPin 36   // Analog pin for water level sensor
#define uvSensorPin 34 // Use any available ADC pin on your ESP32
#define WledPin 25   // LED pin for low water level 
#define UVledPin 26   // LED pin for UV SENSOR
#define PWRled 13   // POWER LED
#define UVbutton 27   // Button pin for UV SENSOR
#define trigPin 5   // Trig pin
#define echoPin 19   // Echo pin
#define moistureSensorPin 39  // moMoisture pin
#define moisturebutton 14  // mositure button pin
#define motorpin 5  // Relay pin
#define ledmoisture 12  // LED for low moisture
#define ledmotor 13  // LED for relay on

//THRESHOLDS
const int waterLevelThreshold = 20; // 20% water level
const int uvThreshold = 25; // 25% UV level percentage threshold
const float MAX_UV_VOLTAGE = 3.3;  // Adjust based on your sensor's max output (example: 3.0V)
const float UV_SENSITIVITY = 0.1;  // Sensitivity of GUVA-S12SD (voltage per mW/cm²)
const int maxDistance = 50; // Maximum distance in cm

//TIME LIMITS FOR UV SENSOR ACTIVATION
int startHour = 8;
int startMinute = 30;
int endHour = 16;
int endMinute = 30;

int uvLevel = 0;   // UV sensor value

ThreeWire myWire(IO, SCLK, CE);  // Initializes the ThreeWire interface with specified pins.
RtcDS1302<ThreeWire> Rtc(myWire);  // Creates an RTC object using the ThreeWire interface.

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Creates an LCD object with I2C address 0x27 and 16x2 dimensions.

//TIME AND DATE DISPLAY 

void setup() {
  Serial.begin(115200);  // Starts the serial communication at 115200 baud rate.
  
  lcd.init();  // Initializes the LCD
  lcd.backlight();  // Turns on the LCD backlight for better visibility.
  lcd.clear();  
  lcd.setCursor(3, 0);  // Positions the cursor at the start of the first row.
  lcd.print("Hi,This is");  // Displays an initializing message on the LCD.
  lcd.setCursor(5, 1);  // Positions the cursor at the start of the first row.
  lcd.print("ROOTY!");  // Displays an initializing message on the LCD.

  delay(10000);
  lcd.clear();


  Serial.print("compiled: ");  // Prints the compile date and time to the serial monitor.
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  Rtc.Begin();  // Initializes the RTC module.

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);  // Creates a DateTime object with compile-time values.
  printDateTime(compiled);  // Prints the compile-time date and time.
  Serial.println();

  if (!Rtc.IsDateTimeValid()) {
    Serial.println("RTC lost confidence in the DateTime!");  // Indicates invalid RTC date/time.
    Rtc.SetDateTime(compiled);  // Sets RTC to compile-time date and time.
  }

  if (Rtc.GetIsWriteProtected()) {
    Serial.println("RTC was write protected, enabling writing now");  // Disables RTC write protection if enabled.
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");  // Starts the RTC if it was not running.
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();  // Gets the current date and time from the RTC.
  if (now < compiled) {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");  // Updates RTC if it is older than compile time.
    Rtc.SetDateTime(compiled);
  } else if (now > compiled) {
    Serial.println("RTC is newer than compile time. (this is expected)");  // Indicates RTC time is correct.
  } else if (now == compiled) {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");  // Handles rare case when RTC matches compile time.
  }

// WATER LEVEL SENSOR AND LED
  pinMode(waterSensorPin, INPUT); // Define the water sensor pin as INPUT
  pinMode(WledPin, OUTPUT); // Define the LED pin as OUTPUT
  Serial.begin(115200); // Initialize serial communication for debugging

// UV SENSOR
  pinMode(UVledPin, OUTPUT); // Initialize LED pin
  pinMode(UVbutton, INPUT_PULLUP); // Initialize button pin
  Serial.begin(115200);  // Initialize serial communication
  pinMode(uvSensorPin, INPUT);  // Set the UV sensor pin as input

// US SENSOR
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

// MOISTURE SENSOR
  pinMode(moisturebutton, INPUT_PULLUP);
  pinMode(moistureSensorPin, INPUT);
  pinMode(motorpin, OUTPUT);
  pinMode(ledmoisture, OUTPUT);
  pinMode(ledmotor, OUTPUT);
  digitalWrite(motorpin, LOW);  // Ensure relay starts off
  digitalWrite(ledmoisture, LOW); // Ensure LEDs start off
  digitalWrite(ledmotor, LOW);

}

void loop() {
  RtcDateTime now = Rtc.GetDateTime();  // Gets the current date and time from the RTC.

  displayDateTime(now);  // Displays the current date and time on the LCD.

  if (!now.IsValid()) {
    Serial.println("RTC lost confidence in the DateTime!");  // Logs invalid date/time error.
    lcd.setCursor(0, 1);  // Sets the cursor to the second row on the LCD.
    lcd.print("Invalid DateTime");  // Displays an error message on the LCD.
  }

  delay(1000);  // Waits one second before updating.

// WATER LEVEL SENSOR AND LED
  int waterLevel = analogRead(waterSensorPin); // Read water level sensor
  int waterLevelPercentage = map(waterLevel, 0, 4095, 0, 100); // Map to percentage

  Serial.print("Water Level: ");
  Serial.print(waterLevelPercentage);
  Serial.println("%");

  if (waterLevelPercentage < waterLevelThreshold) {
    digitalWrite(WledPin, HIGH); // Turn on LED if water level is below threshold
  } else {
    digitalWrite(WledPin, LOW);  // Turn off LED if water level is above threshold
  }

  delay(1000);  // Wait for 1 second before next reading

//UV SENSOR PROCESS
  // Read analog value from UV sensor
  uvLevel = analogRead(uvSensorPin);

  // Convert analog value to voltage
  float voltage = uvLevel * (3.3 / 4095.0);

  // Calculate UV intensity (mW/cm²)
  float uvIntensity = voltage / UV_SENSITIVITY;

  // Calculate UV percentage
  float uvPercentage = (voltage / MAX_UV_VOLTAGE) * 100.0;
  uvPercentage = constrain(uvPercentage, 0, 100);

  // Get current time
  int currentHour = now.Hour();
  int currentMinute = now.Minute();

  // Check if within time range and UV percentage
  if ((currentHour > startHour || (currentHour == startHour && currentMinute >= startMinute)) &&
      (currentHour < endHour || (currentHour == endHour && currentMinute <= endMinute))) {
    if (uvPercentage < 20.0) {
      digitalWrite(UVledPin, HIGH);  // Turn on LED
    } else {
      digitalWrite(UVledPin, LOW);   // Turn off LED
    }
  } else {
    digitalWrite(UVledPin, LOW);  // Turn off LED outside time range
  }

  // Display UV percentage on button press
  if (digitalRead(UVbutton) == LOW) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("UV Level:");
    lcd.setCursor(0, 1);
    lcd.print(uvPercentage, 1);
    lcd.print("%");
    delay(1000);  // Show display for 10 seconds
    lcd.clear();
  }

  // Serial output for debugging
  Serial.print("Analog Value: ");
  Serial.print(uvLevel);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 2);
  Serial.print(" V | UV Intensity: ");
  Serial.print(uvIntensity, 2);
  Serial.print(" mW/cm² | UV Percentage: ");
  Serial.print(uvPercentage, 1);
  Serial.println("%");

  delay(1000);  // Delay before next reading

//US SENSOR PROCESS

  // Send a pulse to the trigPin to start the ultrasonic measurement
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echoPin to get the duration of the pulse
  long duration = pulseIn(echoPin, HIGH);

  // Calculate the distance in centimeters
  float distance = (duration / 2.0) * 0.0343;

  // Ensure the distance is within the valid range (0 to 50 cm)
  if (distance <= maxDistance && distance >= 0) {
    // Calculate (50 - current reading)
    float result = maxDistance - distance;
    Serial.print("Plant Height: ");
    Serial.print(result);
    Serial.println(" cm");
  } else {
    // If distance is out of range, print an error
    Serial.println("Out of range");
  }

  // Wait a bit before the next measurement
  delay(500);

//SOIL MOISTURE PROCESS
  static bool buttonPressed = false;
  int buttonState = digitalRead(moisturebutton);

  // Check if the button is pressed
  if (buttonState == LOW && !buttonPressed) {
    buttonPressed = true;
    int moistureLevel = analogRead(moistureSensorPin);
    float moisturePercentage = map(moistureLevel, 0, 4095, 0, 100);

    // Display moisture level on LCD
    lcd.setCursor(0, 1);
    lcd.print("Moisture: ");
    lcd.print(moisturePercentage);
    lcd.print("%   ");

    // Print moisture level to Serial Monitor
    Serial.print("Moisture Level: ");
    Serial.print(moisturePercentage);
    Serial.println("%");

    delay(500);  // Debounce delay
  } else if (buttonState == HIGH) {
    buttonPressed = false;
  }

  // Check time range for relay control
  if ((currentHour >= 6 && currentHour < 8) || (currentHour >= 14 && currentHour < 18)) {
    int moistureLevel = analogRead(moistureSensorPin);
    float moisturePercentage = map(moistureLevel, 0, 4095, 0, 100);

    if (moisturePercentage < 20) {
      digitalWrite(ledmoisture, HIGH); // Turn on moisture LED
      lcd.setCursor(0, 1);
      lcd.print("Watering...");
      Serial.println("Turning on relay for 10 seconds.");
      digitalWrite(motorpin, HIGH);
      digitalWrite(ledmotor, HIGH);  // Turn on relay LED
      delay(10000);  // Relay ON for 10 seconds
      digitalWrite(motorpin, LOW);
      digitalWrite(ledmotor, LOW);   // Turn off relay LED
    } else {
      digitalWrite(ledmoisture, LOW); // Turn off moisture LED if above threshold
    }
  }

  delay(100);  // Loop delay to reduce CPU usage
}

// Function to display UV level on I2C display for 10 seconds
void displayUVLevel(int uvLevelPercentage) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UV Level: ");
  lcd.print(uvLevelPercentage);  // Display UV level percentage on screen
  lcd.print(" %");
  
  delay(10000);  // Display for 10 seconds
  lcd.clear();
}

#define countof(a) (sizeof(a) / sizeof(a[0]))  // Defines a macro to calculate the size of an array.

void printDateTime(const RtcDateTime& dt) {
  char datestring[20];  // Buffer to store the formatted date/time string.

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second());
  Serial.print(datestring);  // Prints the formatted date/time to the serial monitor.
}

void displayDateTime(const RtcDateTime& dt) {
  lcd.setCursor(0, 0);  // Positions the cursor at the start of the first row.
  char datestring[20];  // Buffer to store the formatted date string.

  snprintf_P(datestring,
             countof(datestring),
             PSTR("DATE: %02u/%02u/%04u"),
             dt.Month(),
             dt.Day(),
             dt.Year());
  lcd.print(datestring);  // Displays the formatted date on the LCD.

  lcd.setCursor(0, 1);  // Positions the cursor at the start of the second row.
  char timestring[15];  // Buffer to store the formatted time string.

  snprintf_P(timestring,
             countof(timestring),
             PSTR("TIME: %02u:%02u"),
             dt.Hour(),
             dt.Minute());
  lcd.print(timestring);  // Displays the formatted time on the LCD.
}



