  /*
 * Este código tem como objetivo controlar uma fechadura de porta HDL FEC-91
 * por meio do uso de cartões de acesso, que nesse caso serão os cartões de
 * identificação estudantil. 
 * 
 * Os componentes de hardware são os que seguem:
 * - 1 Leitor RFID ~ MFRC-522
 * - 1 LCD interfaceado por protocolo de comunicação I2C
 * - 1 Fechadura HDL FEC-91 
 * - 1 Relé
 * - 1 Fonte 12V
 * 
 * Pela limitação de memória flash do Arduino, a capacidade de armazenamento é 
 * restrita, no momento atendendo somente a um máximo de 30 usuários cadastrados.
 * 
 * Modo de reconhecimento:
 * 
 * Ao aproximar uma das chaves-mestre duas vezes seguidas no leitor, o LCD
 * indicará que foi iniciado o modo de reconhecimento do dispositivo. Após
 * o início deste modo, basta aproximar o cartão que deseja adicionar, e 
 * um aviso no LCD indicará o sucesso da operação. Se for aproximado um 
 * cartão já presente no sistema, este será removido, com devida indicação
 * no painel LCD. O modo de reconhecimento dura 5 segundos, sendo reiniciado
 * a cada adição de novo cartão. Os cartões ficam de forma permanente, não
 * sendo necessária a re-adição deles em caso de posterior adição de novos
 * usuários.
 * 
 * 
 * Método de acionamento do sistema:
 * 
 * Ao conectar a fonte específica da fechadura na tomada, e energizar
 * a placa, o sistema já estará em funcionamento, em repouso. 
 * OBS: A fonte 12V da fechadura pode ser substituída por pilhas, ou 
 * qualquer análogo que resulte na tensão mínima necessária. Já o
 * circuito basta 9V para acionar.
 * 
 * 
 * 
 * 
 */


#include <SPI.h>
#include <MFRC522Extended.h>
#include <deprecated.h>
#include <require_cpp11.h>
#include <MFRC522.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SS_PIN 10
#define RST_PIN 9 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

MFRC522 rfid(SS_PIN, RST_PIN);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


String authorized_uids[30]; // Lista para armazenar até 20 UIDs autorizados
int authorized_uids_count = 0;

const char* texto = "Oficina de Computadores"; // Texto que queremos rolar
int textLength = strlen(texto) * 18; // Comprimento do texto ajustado para tamanho 3
int x = SCREEN_WIDTH; // Começa fora da tela à direita

unsigned long previousMillis = 0; // Armazena o último tempo em que o texto foi atualizado
const long interval = 0; //Intervalo de exibição das letras

unsigned long statusMillis = 0;
int statusDuration = 2000; // Duração da mensagem de status em milissegundos
int statusMessage = 0; // Armazena o status atual a ser exibido

String uid_mestre1 = "40e8f736"; // Tag azul
String uid_mestre2 = "8dbc2d5b"; // Cartão branco

String uidString; // Variável para armazenar o UID
String lastUid; // Variável para armazenar o último UID lido
unsigned long lastUidTime = 0; // Tempo em que o último UID foi lido

const int rele_pin = 7;
const int buzzer = 6;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(rele_pin, OUTPUT); // Define o pino do relé como saída
  digitalWrite(rele_pin, HIGH);

  pinMode(buzzer, OUTPUT); // Define o buzzer
  digitalWrite(buzzer, HIGH);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  
  Serial.println("Aproxime o cartão");
  Serial.println();
}

void loop() {
  show_idle_message();
  
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  get_uid();

  // Verificar se o UID lido é igual ao UID autorizado
  if (isAuthorized(uidString)) {
    show_status_message(1);
    libera_entrada(); 
  } else if (uidString.equalsIgnoreCase(uid_mestre1) || uidString.equalsIgnoreCase(uid_mestre2)) {
    unsigned long currentTime = millis();
    if (uidString.equalsIgnoreCase(lastUid) && (currentTime - lastUidTime < 5000)) {
      Serial.println("Modo de reconhecimento iniciado...");
      show_status_message(5);
      delay(1000);
      modo_reconhecimento();
    } else {
      Serial.println("UID mestre lido. Aproxime novamente dentro de 5 segundos para iniciar o modo de reconhecimento.");
      show_status_message(6);
    }
    lastUid = uidString;
    lastUidTime = currentTime;
  } else {
    show_status_message(2);
    recusa_entrada();
  }

  delay(500);
  
  // Para a leitura do cartão
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
/*
 *                0 1 2 3 4 5 6 7 8 9 1 2 3 4 5
 *              0     O f i c i n a   d e 
 *              1   C o m p u t a d o r e s 
 *
 *
 *                0 1 2 3 4 5 6 7 8 9 1 2 3 4 5
 *              0       E n t r a d a 
 *              1       A u t o r i z a d a
 *              
 *              
 *                0 1 2 3 4 5 6 7 8 9 1 2 3 4 5
 *              0       E n t r a d a
 *              1       R e c u s a d a
 *              
 *              
 *                0 1 2 3 4 5 6 7 8 9 1 2 3 4 5
 *              0       U s u á r i o
 *              1       A d i c i o n a d o
 *              
 *              
 *                0 1 2 3 4 5 6 7 8 9 1 2 3 4 5
 *              0       U s u á r i o
 *              1       R e m o v i d o             
 *              
 *              
 *                0 1 2 3 4 5 6 7 8 9 1 2 3 4 5
 *              0       M o d o   d e
 *              1 R e c o n h e c i m e n t o             
 *              
 *              
 *                0 1 2 3 4 5 6 7 8 9 1 2 3 4 5
 *              0       A p r o x i m e
 *              1       N o v a m e n t e
 *              
 *              
 *                0 1 2 3 4 5 6 7 8 9 1 2 3 4 5
 *              0 X X X X X X X X X X X X X X X
 *              1 X X X X X X X X X X X X X X X              
 */


void show_idle_message() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    display.clearDisplay();
    display.setCursor(x, 8);
    display.println(texto);
    display.display();

    x--; 

    if (x < -textLength) { 
      x = SCREEN_WIDTH;
    }
  }
}

void show_status_message(int status) {
  unsigned long currentMillis = millis();
  
  if (currentMillis - statusMillis >= statusDuration || statusMessage != status) {
    statusMillis = currentMillis;
    statusMessage = status;
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    switch(status) {
      case 1: 
        display.println("Entrada");
        display.setCursor(0, 16);
        display.println("Autorizada");
        break;

      case 2:
        display.println("Entrada");
        display.setCursor(0, 16);
        display.println("Recusada");
        break;

      case 3:
        display.println("Usuário");
        display.setCursor(0, 16);
        display.println("Adicionado");
        break;

      case 4:
        display.println("Usuário");
        display.setCursor(0, 16);
        display.println("Removido");
        break;

      case 5:
        display.println("Modo de");
        display.setCursor(0, 16);
        display.println("Reconhecimento");
        break;

      case 6: 
        display.println("Aproxime");
        display.setCursor(0, 16);
        display.println("Novamente");
        break;
    }

    display.display();
  }
}

void libera_entrada() {
  Serial.println("UID autorizado. Abrindo fechadura...");
  digitalWrite(buzzer, LOW); // Liga o buzzer
  digitalWrite(rele_pin, LOW); // Ativa o relé para abrir a fechadura
  delay(500); // Mantem todos os sistemas ativos por 500ms
  digitalWrite(rele_pin, HIGH); // Desativa o relé
  digitalWrite(buzzer, HIGH); // Desativa o buzzer
}

void recusa_entrada() {
  Serial.println("UID não autorizado.");
  //digitalWrite(red_led, HIGH); 
  digitalWrite(buzzer, LOW);
  delay(500);
  //digitalWrite(red_led, LOW);
  digitalWrite(buzzer, HIGH);
}

void get_uid() {
  uidString = "";

  // Exibir e armazenar o UID do cartão
  Serial.print("UID do cartão: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      uidString += "0";
    }
    uidString += String(rfid.uid.uidByte[i], HEX); // Atualiza a uidString com a concatenação de cada dígito do uid
  }
  Serial.println(uidString);
}

// Realiza a verificação dos cartões autorizados para liberar entrada
bool isAuthorized(String uid) {
  for (int i = 0; i < authorized_uids_count; i++) {
    if (uid.equalsIgnoreCase(authorized_uids[i])) {
      return true;
    }
  }
  return false;
}

// Adiciona o cartão ao sistema
void add_uid(String uid) {
  if (authorized_uids_count < 20) {
    authorized_uids[authorized_uids_count++] = uid;
    Serial.println("UID adicionado à lista de autorizados.");
  } else {
    Serial.println("Lista de UIDs autorizados está cheia.");
  }
}

// Remove o cartão do sistema
void remove_uid(String uid) {
  for (int i = 0; i < authorized_uids_count; i++) {
    if (uid.equalsIgnoreCase(authorized_uids[i])) {
      for (int j = i; j < authorized_uids_count - 1; j++) {
        authorized_uids[j] = authorized_uids[j + 1];
      }
      authorized_uids_count--;
      Serial.println("UID removido da lista de autorizados.");
      return;
    }
  }
  Serial.println("UID não encontrado na lista de autorizados.");
}

// Define o modo de reconhecimento por apresentação do cartão mestre
void modo_reconhecimento() {
  //digitalWrite(blue_led, HIGH);
  Serial.println("Modo de reconhecimento iniciado. Aproxime um novo cartão para adicionar ou remova um existente.");
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) { // Modo de reconhecimento ativo por 5 segundos
    if (!rfid.PICC_IsNewCardPresent()) continue;
    if (!rfid.PICC_ReadCardSerial()) continue;

    get_uid();

    // Adição/remoção de usuários
    if (isAuthorized(uidString)) {
      show_status_message(4);
      remove_uid(uidString);
      //digitalWrite(red_led, HIGH);
      delay(500);
      //digitalWrite(red_led, LOW);
    } else {
      show_status_message(3);
      add_uid(uidString);
      //digitalWrite(green_led, HIGH);
      delay(500);
      //digitalWrite(green_led, LOW);
    }

    delay(1000);

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    startTime = millis(); // Reinicia o tempo de reconhecimento
  }
  Serial.println("Modo de reconhecimento encerrado.");
  //digitalWrite(blue_led, LOW);
}
