const int rele_pin = 7;

void setup() {
  pinMode(rele_pin, OUTPUT); //Define o Input do relé
  digitalWrite(rele_pin, HIGH);
}

void loop() {
  digitalWrite(rele_pin, LOW);
  delay(500);
  digitalWrite(rele_pin, HIGH);
  delay(5000);
}
