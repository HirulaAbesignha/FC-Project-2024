#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin definitions
const int soilMoisturePin = 36;  // Analog pin for the soil moisture sensor
const int motorPin = 16;         // GPIO16 (RX2) for motor pump control
const int airValue = 4095;       // Sensor value when soil is dry
const int waterValue = 0;        // Sensor value when soil is fully wet
const int moistureThreshold = 20; // Threshold percentage for soil moisture

// Ultrasonic sensor pins
const int triggerPin = 5;  // Trigger pin for ultrasonic sensor
const int echoPin = 18;     // Echo pin for ultrasonic sensor

unsigned long previousMillis = 0;  // Store the time for alternating screens
const long interval = 2000;         // Interval to alternate screens (2 seconds)

void setup() {
  lcd.init();              // Initialize the LCD
  lcd.backlight();         // Turn on the backlight
  lcd.setCursor(0, 0);
  lcd.print("Soil Moisture:");

  pinMode(motorPin, OUTPUT);   // Set motor pin as output
  digitalWrite(motorPin, LOW); // Ensure motor is off initially
  
  pinMode(triggerPin, OUTPUT); // Set ultrasonic trigger pin as output
  pinMode(echoPin, INPUT);     // Set ultrasonic echo pin as input
  
  Serial.begin(115200);    // Initialize serial communication for debugging
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if it's time to switch between screens
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Display Soil Moisture and Motor Status
    displaySoilMoistureAndMotor();

    delay(1000); // Wait for 1 second before switching display

    // Display Ultrasonic sensor reading
    displayUltrasonicDistance();
    
    delay(1000); // Wait for 1 second before switching display
  }
}

void displaySoilMoistureAndMotor() {
  int sensorValue = analogRead(soilMoisturePin); // Read the sensor value
  int moisturePercent = map(sensorValue, airValue, waterValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100); // Ensure the value is between 0 and 100
  
  // Control motor based on soil moisture percentage
  if (moisturePercent < moistureThreshold) {
    digitalWrite(motorPin, HIGH); // Turn on the motor
  } else {
    digitalWrite(motorPin, LOW);  // Turn off the motor
  }
  
  // Display soil moisture percentage
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");
  lcd.print(moisturePercent);
  lcd.print("%   ");  // Spaces to clear previous characters

  // Display motor status
  lcd.setCursor(0, 1); // Reset cursor to second line
  lcd.print("Motor: ");
  if (digitalRead(motorPin) == HIGH) {
    lcd.print("ON  "); // Print "ON"
  } else {
    lcd.print("OFF "); // Print "OFF"
  }
  
  // Debugging info in Serial Monitor
  Serial.print("Raw Value: ");
  Serial.print(sensorValue);
  Serial.print(" | Moisture: ");
  Serial.print(moisturePercent);
  Serial.print("% | Motor: ");
  Serial.println(digitalRead(motorPin) == HIGH ? "ON" : "OFF");
}

void displayUltrasonicDistance() {
  // Trigger the ultrasonic sensor to send a pulse
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  // Measure the duration of the pulse
  long duration = pulseIn(echoPin, HIGH);

  // Calculate the distance based on the speed of sound (343 m/s)
  long distance = duration * 0.034 / 2;  // Distance in cm

  // Display ultrasonic distance
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Distance: ");
  lcd.print(distance);
  lcd.print(" cm");

  // Debugging info in Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
}
