/*
const int moistureSensorPin = 13; 

void setup() {
  Serial.begin(115200);
}
void loop() {
  
  int moistureLevel = analogRead(moistureSensorPin);

  Serial.print("Moisture Level: ");
  Serial.println(moistureLevel);

  delay(1000);
}
*/

const int moistureSensorPin = 34;
const int pumpPin = 13;
const int moistureThreshold = 500;

void setup() {
  Serial.begin(115200);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);
}

void loop() {
  int moistureLevel = analogRead(moistureSensorPin);
  Serial.print("Moisture Level: ");
  Serial.println(moistureLevel);
  if (moistureLevel > moistureThreshold) {
    Serial.println("Pump is ON");
    digitalWrite(pumpPin, HIGH);
  } else {
    Serial.println("Pump is OFF");
    digitalWrite(pumpPin, LOW);
  }
  delay(1000);
}

/*
// Define the pin number for the LED
const int ledPin = 13; // Most Arduino boards have an onboard LED on pin 13

void setup() {
  // Initialize the LED pin as an output
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // Turn the LED on (HIGH is the voltage level)
  digitalWrite(ledPin, HIGH);
  // Wait for 1 second (1000 milliseconds)
  delay(1000);

  // Turn the LED off by making the voltage LOW
  digitalWrite(ledPin, LOW);
  // Wait for 1 second
  delay(1000);
}
*/