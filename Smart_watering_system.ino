/*
 * Enhanced Moisture Monitor with Auto Pump Control
 * Pump turns OFF automatically when moisture >= 60%
 */

const int MOISTURE_PIN = A0;
const int RELAY_PIN = 7;
const int ALARM_LED = 13;  // Built-in LED

// Safety thresholds
const int MAX_RUN_TIME = 300000;  // 5 minutes max (for safety)
unsigned long pumpStartTime = 0;
bool pumpOverride = false;

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(ALARM_LED, OUTPUT);
  
  // Start with pump ON
  digitalWrite(RELAY_PIN, LOW);
  pumpStartTime = millis();
  
  Serial.println("System started - Pump is ON");
  Serial.println("Send 'x' to toggle pump (override), 'r' for reading");
}

void loop() {
  // Read moisture sensor
  int moisture = analogRead(MOISTURE_PIN);
  int percent = map(moisture, 620, 280, 0, 100);
  percent = constrain(percent, 0, 100);

  // Display current status every 2 seconds
  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay > 2000) {
    lastDisplay = millis();
    Serial.print("Moisture: ");
    Serial.print(moisture);
    Serial.print(" (");
    Serial.print(percent);
    Serial.print("%) | Pump: ");
    Serial.println(digitalRead(RELAY_PIN) == LOW ? "ON" : "OFF");
  }

  // ----------- AUTO PUMP CONTROL BASED ON MOISTURE ---------------
  if (!pumpOverride) {
    if (percent >= 60) {
      // Soil is wet enough → Pump OFF
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(ALARM_LED, LOW);
      pumpStartTime = millis(); // Reset timer because pump is off
    } else {
      // Soil dry → Pump ON
      if (digitalRead(RELAY_PIN) == HIGH) {
        pumpStartTime = millis(); // Pump just turned on
      }
      digitalWrite(RELAY_PIN, LOW);
    }
  }

  // ------------- SAFETY TIMER (5 MIN MAX RUNTIME) ----------------
  if (!pumpOverride && digitalRead(RELAY_PIN) == LOW &&
      millis() - pumpStartTime > MAX_RUN_TIME) {

    digitalWrite(RELAY_PIN, HIGH); // Pump OFF
    digitalWrite(ALARM_LED, HIGH); // Alarm LED ON

    Serial.println("SAFETY: Pump turned OFF after 5 minutes!");
    Serial.println("Send 'x' to override and turn pump back ON");
  }

  // ------------------- SERIAL COMMAND HANDLING -------------------
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == 'x' || cmd == 'X') {
      // Toggle pump & enable override
      pumpOverride = true;
      bool state = digitalRead(RELAY_PIN);
      digitalWrite(RELAY_PIN, !state); 
      digitalWrite(ALARM_LED, LOW);

      Serial.println(state ? "Pump turned ON (override)" : "Pump turned OFF (override)");
    }

    else if (cmd == 'r' || cmd == 'R') {
      Serial.print("Quick read: ");
      Serial.println(analogRead(MOISTURE_PIN));
    }
  }

  delay(100);
}
