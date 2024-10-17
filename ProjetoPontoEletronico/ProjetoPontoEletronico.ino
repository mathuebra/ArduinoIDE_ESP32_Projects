//Inclusão das bibliotecas para comunicação WiFi
#include <WiFiAP.h>
#include <WiFiScan.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiGeneric.h>
#include <WiFiSTA.h>
#include <WiFiMulti.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFiType.h>

#include <ThreeWire.h>

//RTC DS1302
#include <RtcTemperature.h>
#include <RtcUtility.h>
#include <RtcDS1302.h>
#include <RtcDS3231.h>
#include <EepromAT24C32.h>
#include <RtcDateTime.h>
#include <RtcDS3234.h>
#include <RtcDS1307.h>

//RFID MFRC522
#include <MFRC522Extended.h>
#include <deprecated.h>
#include <require_cpp11.h>
#include <MFRC522.h>

//LCD_I2C
#include <LiquidCrystal_I2C.h>

/*
 * Pinagem dos módulos:
 * 
 * rfid: MISO - 19
 *       MOSI - 23
 *       SCK - 18
 *       SS - 5
 *       3.3v
 *       GND
 *       RST
 * 
 * 
 * 
 */

MFRC522 rfid(SS_PINOUT, RST_PINOUT); //pinout
LiquidCrystal_I2C lcd(ADDRESS, 16, 2); //pinout
ThreeWire wireClockModule(DATE_PINOUT, CLOCK_PINOUT, RST_PINOUT); //pinout
RtcDS1302<ThreeWire> rtc(wireClockModule); //pinout

const char* networkName = "Cefast Baja";
const char* networkPassword = "*cefastbaja#2023";

void setup() {

  Serial.begin(115200);

  WiFi.begin(networkName, networkPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");
  
  SPI.begin();
  rfid.PCD_Init();
  printf("Aproxime o cartão: \r\n");

  //PROCESSO DE INICIALIZAÇÃO DO LCD
  lcd.init();
  lcd.backlight();
  //FINAL DA INICIALIZAÇÃO DO LCD

  //PROCESSO DE INICIALIZAÇÃO DO DS1302
  rtc.begin();
  
  Serial.print("Compilado em: ");
  
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__); //VARIÁVEL RECEBE DATA E HORA DE COMPILAÇÃO
  printDateTime(compiled); //PASSA OS PARÂMETROS PARA A FUNÇÃO printDateTime
  Serial.println(); 
  Serial.println(); 
  
    if(Rtc.GetIsWriteProtected()){ 
      Serial.println("O módulo está com proteção de escrita, desabilitando...");
      Rtc.SetIsWriteProtected(false); 
      Serial.println(); 
  }
    if(!Rtc.GetIsRunning()){ 
      Serial.println("O módulo não está rodando, inciando processos..."); 
      Rtc.SetIsRunning(true); 
      Serial.println(); 
  }
  
  RtcDateTime now = Rtc.GetDateTime(); 
  if (now < compiled) { 
      Serial.println("O tempo de compilação em comparação com o horário está incorreto, corrigindo...");
      Rtc.SetDateTime(compiled); 
      Serial.println(); 
  } else if (now > compiled){
      Serial.println("O tempo recebido é mais recente que o tempo de compilação, funcionamento correto do módulo detectado.");
      Serial.println(); 
  } else if (now == compiled) { 
      Serial.println("O tempo recebido é igual ao tempo de compilação, funcionamento correto do módulo detectado.");
      Serial.println(); 
  }
  //FINAL DA INICIALIZAÇÃO DO DS1302
}

void loop() {

  String strID = "";
  unsigned long uid;
  RtcTimeDate now = rtc.GetDateTime(); //Tempo atual
  
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("%02u/%02u/%04u", now.Day(), now.Month(), now.Year());
  lcd.setCursor(0,1);
  lcd.print("%02u:%02u:%02u", now.Hour(), now.Minute(), now.Second());

  
  if (!rfid.PICC_IsNewCardPresent()) //Caso não encontre nenhuma entrada no RFID, reinicia o loop e continua procurando
    return;

  printf("Hora : %02u/%02u/%04u \r\n", now.Day(), now.Month(), now.Year());
  
  //Se chegou aqui existe um cartão presente, logo, extrai o uid do cartão
  uid = getID();
    
    // TODO:
    // Verificar a possibilidade de usar a flash do ESP para salvar tentativas de acesso ao invés do SD (n sei se vai ter q usar esp-idf)
    // Salvar esses dados no cartao SD em um esquema de stack 
    // Conectar no wifi do baja
    // Enviar mensagens com http no servidor
    // Enviar os dados a medida que conseguir conectar 
    
}

unsigned long getID(){

  // Verifica se é possível ler os dados do cartão
  if (!rfid.PICC_ReadCardSerial())
    return 0;
  
  unsigned long hex_num = 0x0000;
  
  // Numero salvo em big endian
  hex_num =  mfrc522.uid.uidByte[0] << 24;
  hex_num += mfrc522.uid.uidByte[1] << 16;
  hex_num += mfrc522.uid.uidByte[2] <<  8;
  hex_num += mfrc522.uid.uidByte[3];
  mfrc522.PICC_HaltA(); // Stop reading

  printf("Codigo do cartao lido : %x \r\n",hex_num);
  return hex_num;
}
