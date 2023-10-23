void setup() {
  // Initialize the serial communication:
  Serial.begin(9600);
  pinMode(10, INPUT); // Setup for leads off detection LO +
  pinMode(11, INPUT); // Setup for leads off detection LO -
}

void loop() {
  if (digitalRead(10) == 1 || digitalRead(11) == 1) {
    Serial.println('!'); // Leads off detected
  } else {
    // Send the value of analog input 0:
    Serial.println(analogRead(A0));
  }
  // Wait for a bit to prevent serial data from saturating
  delay(1000);
}
