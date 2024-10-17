// Define o pino do buzzer
const int buzzerPin = 23;  // Pino do buzzer

void setup() {
  // Configura o pino do buzzer como saída
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, HIGH);
}

void loop() {
  digitalWrite(buzzerPin, LOW);
  delay(1000);
  digitalWrite(buzzerPin, HIGH);
  delay(1000);
  
}
