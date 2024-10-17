  /*
 * Este código tem como objetivo controlar uma fechadura de porta HDL FEC-91
 * por meio do uso de cartões de acesso, que nesse caso serão os cartões de
 * identificação estudantil. 
 * 
 * Componentes de hardware:
 * - 1 Leitor RFID ~ MFRC-522
 * - 1 LCD interfaceado por protocolo de comunicação I2C
 * - 1 Fechadura HDL FEC-91 
 * - 1 Relé
 * - 1 ESP32
 * - 1 Fonte 12V
 * 
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
 * qualquer análogo que resulte na tensão mínima necessária. 
 * 
 * 
 * 
 * 
 */

#include <SPI.h>
//#include <MFRC522Extended.h>
#include <MFRC522.h>

#include <LiquidCrystal_I2C.h>

#define SS_PIN 5
#define RST_PIN 4
#define LCD_ADDRESS 0x3F

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);

String authorized_uids[30];
int authorized_uids_count = 0;

String uid_mestre1 = "40e8f736";
String uid_mestre2 = "8dbc2d5b";

String uidString;
String lastUid;
unsigned long lastUidTime = 0;

//analisar dps
const int rele_pin = 13;
//const int buzzer = 6;

const int buttonPin = 12;
const int builtInLed = 2;

int buttonState = 0;

void setup() {
    Serial.begin(115200);
    SPI.begin();
    rfid.PCD_Init();

    lcd.init();
    lcd.clear();
    lcd.backlight();

    pinMode(rele_pin, OUTPUT);
    digitalWrite(rele_pin, HIGH);

    //pinMode(buzzer, OUTPUT);
    //digitalWrite(buzzer, HIGH);

    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(builtInLed, OUTPUT);

    Serial.println("Aproxime o cartão");
    Serial.println();
}

void loop() {
    show_idle_message();

    buttonState = digitalRead(buttonPin);

    if (buttonState == LOW) libera_entrada();

    if(!rfid.PICC_IsNewCardPresent()) return;
    if(!rfid.PICC_ReadCardSerial()) return;

    get_uid();

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
 *                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 *              0       O f i c i n a   d e 
 *              1     C o m p u t a d o r e s 
 *
 *
 *                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 *              0       E n t r a d a 
 *              1       A u t o r i z a d a
 *              
 *              
 *              
 *                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 *              0       S a í d a
 *              1       A u t o r i z a d a
 *              
 *              
 *              
 *                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 *              0       E n t r a d a
 *              1       R e c u s a d a
 *              
 *              
 *                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 *              0       U s u á r i o
 *              1       A d i c i o n a d o
 *              
 *              
 *                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 *              0       U s u á r i o
 *              1       R e m o v i d o             
 *              
 *              
 *                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 *              0         M o d o   d e
 *              1   R e c o n h e c i m e n t o             
 *              
 *              
 *                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 *              0       A p r o x i m e
 *              1       N o v a m e n t e
 *              
 *              
 *                0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 *              0 X X X X X X X X X X X X X X X
 *              1 X X X X X X X X X X X X X X X              
 */

void show_idle_message() {
    lcd.setCursor(3, 0);
    lcd.print("Oficina de");
    lcd.setCursor(2, 1);
    lcd.print("Computadores");
}

void show_status_message(int status) {
    lcd.clear();

    switch(status) {
      case 1: 
        lcd.setCursor(3, 0);
        lcd.print("Entrada");
        lcd.setCursor(3, 1);
        lcd.print("Autorizada");
        break;

      case 2:
        lcd.setCursor(3, 0);
        lcd.print("Entrada");
        lcd.setCursor(3, 1);
        lcd.print("Recusada");
        break;

      case 3:
        lcd.setCursor(3, 0);
        lcd.print("Usuário");
        lcd.setCursor(3, 1);
        lcd.print("Adicionado");
        break;

      case 4:
        lcd.setCursor(3, 0);
        lcd.print("Usuário");
        lcd.setCursor(3, 1);
        lcd.print("Removido");
        break;

      case 5:
        lcd.setCursor(4, 0);
        lcd.print("Modo de");
        lcd.setCursor(1, 1);
        lcd.print("Reconhecimento");
        break;

      case 6: 
        lcd.setCursor(3, 0);
        lcd.print("Aproxime");
        lcd.setCursor(3, 1);
        lcd.print("Novamente");
        break;

      case 7:
        lcd.setCursor(3, 0);
        lcd.print("Saida");
        lcd.setCursor(3, 1);
        lcd.print("Liberada");
        break;
    }
}

void libera_entrada() {
  Serial.println("UID autorizado. Abrindo fechadura...");
  //digitalWrite(buzzer, LOW); // Liga o buzzer
  digitalWrite(rele_pin, LOW); // Ativa o relé para abrir a fechadura
  delay(500); // Mantem todos os sistemas ativos por 500ms
  digitalWrite(rele_pin, HIGH); // Desativa o relé
  //digitalWrite(buzzer, HIGH); // Desativa o buzzer
}

void recusa_entrada() {
  Serial.println("UID não autorizado.");
  //digitalWrite(red_led, HIGH); 
  //digitalWrite(buzzer, LOW);
  delay(500);
  //digitalWrite(red_led, LOW);
  //digitalWrite(buzzer, HIGH);
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

bool isAuthorized(String uid) {
  for (int i = 0; i < authorized_uids_count; i++) {
    if (uid.equalsIgnoreCase(authorized_uids[i])) {
      return true;
    }
  }
  return false;
}

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
