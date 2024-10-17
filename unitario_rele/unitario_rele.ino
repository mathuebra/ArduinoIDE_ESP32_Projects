const int rele_pin = 23;

void setup() {
  pinMode(rele_pin, OUTPUT);
  digitalWrite(rele_pin, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(rele_pin, LOW);
  delay(500);
  digitalWrite(rele_pin, HIGH);
  delay(5000);
}
