// Define os pinos
const int buttonPin = 4;    // Pino do push button
const int ledPin = 2;       // Pino do LED embutido

// Variável para armazenar o estado do botão
int buttonState = 0;

void setup() {
  // Configura o pino do botão como entrada
  pinMode(buttonPin, INPUT_PULLUP);

  // Configura o pino do LED como saída
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // Lê o estado do botão
  buttonState = digitalRead(buttonPin);

  // Se o botão for pressionado, o LED acende
  if (buttonState == LOW) { // LOW porque usamos INPUT_PULLUP
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}
