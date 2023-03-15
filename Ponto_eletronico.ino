#include <SD.h> //Cartão SD

//#include <SSD1306.h>

#include <MFRC522.h> //Leitor RFID
#include <MFRC522Extended.h> //Ampliação das funções de RFID

#include <deprecated.h>
#include <require_cpp11.h>

#include <SPI.h> //Biblioteca do arduíno para comunicação rápida de periféricos

#include <ThreeWire.h> //Define Portas

#include <RtcDS1302.h> //Modelos diferentes de clock
#include <RtcDS1307.h>
#include <RtcDS3231.h>
#include <RtcDS3234.h>
#include <RtcDateTime.h>
#include <RtcTemperature.h>
//#include <RtcUtility.h>

#include <LiquidCrystal_I2C.h> //Comunicação com o LCD pelo I2C

#define RST_PIN 13
#define RST_PIN_RFID 9
#define SS_PIN 10
#define BUZZER_PIN 7
#define SD_PIN 8
#define countof(a) (sizeof(a) / sizeof(a[0]))

LiquidCrystal_I2C lcd(0x27, 16, 2); //Define o LCD com o módulo I2C

MFRC522 mfrc522(SS_PIN, RST_PIN_RFID);  //Cria uma instância do leitor

ThreeWire myWire(4,5,2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire); //Cria o clock com base nos fios declarados no myWire

File myFile;

/*
long membros[40];
char nomeMembros[5][40];


void inicializaMembros() 
{
  for (int i = 0; i < 40; i ++) 
    membros[i] = NULL;
    
  membros[0] = 4294941194;
  strcpy(nomeMembros[0], "Matheus Monteiro Huebra Perdigão");

  membros[1] = 19055;
  strcpy(nomeMembros[1], "Luisa Reis Ribeiro");

  membros[2] = 21103;
  strcpy(nomeMembros[2], "Maria Vitória Rodrigues Peixoto");

  membros[3] = 4294944048;
  strcpy(nomeMembros[3], "Lucas Vieira Metzker");

  membros[4] = 546;
  strcpy(nomeMembros[3], "Gustavo Couto Silva");

}
*/

/*
char comparaCartao (long uid)
{
  for (int i = 0; i < 40; i ++) {
    if (uid == membros[i]) {
      return nomeMembros[i];
    }
  }
}
*/

unsigned long getID() //Pega somente o UID (Unique ID) do cartão lido e "transforma" em int 
{
  if ( ! mfrc522.PICC_ReadCardSerial()) { 
    return -1; //Verifica se há um cartão no leitor
  }
  unsigned long id;
  id =  mfrc522.uid.uidByte[0] << 24;
  id += mfrc522.uid.uidByte[1] << 16;
  id += mfrc522.uid.uidByte[2] <<  8;
  id += mfrc522.uid.uidByte[3];
  mfrc522.PICC_HaltA(); 
  return id;
}


void setup() {

  pinMode(BUZZER_PIN, OUTPUT);

  //Inicializa o LCD
  lcd.init();
  lcd.clear(); //Limpa o LCD
  lcd.backlight(); //Da set no background

  SD.begin(SD_PIN);
  myFile = SD.open("registro_ponto.csv", FILE_WRITE);

  SPI.begin();

  //É capaz desse valor precisar ser alterado devido a troca de Arduino Uno por Heltec Lora32 Wi-fi, estudar o caso
  Serial.begin(9600); //Inicia a comunicação com o PC

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    Rtc.Begin(); //Inicia o Real Time Clock

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    Serial.println();

    if (!Rtc.IsDateTimeValid()) 
    {
        //Geralmente pode ser a bateria
        Serial.println("O módulo RTC está errado, possívelmente devido a bateria ou comprometimento do componente físico");
        Rtc.SetDateTime(compiled);
    }


    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("A escrita do módulo RTC estava protegida, estarei habilitando-a");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("O módulo RTC Não estava rodando, agora está");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("O módulo de clock estava atrasado com relação ao tempo de compilação (incorreto)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("O módulo de clock está adiantado em relação ao tempo de compilação (correto)");
    }
    else if (now == compiled) 
    {
        Serial.println("O módulo de clock está igual em relação ao tempo de compilação (correto)");
    }

    SPI.begin();      // Inicia o SPI
    mfrc522.PCD_Init();   // Inicia o leitor RFID
    delay(4);       // Delay opcional, mas é bom pois certas placas necessitam desse delay
    mfrc522.PCD_DumpVersionToSerial();  // Mostra os detalhes do cartão
    Serial.println(F("Insira o cartão para exibição dos dados:"));
}


void loop() {

char data[20] = "\0";
char horario[20] = "\0";

RtcDateTime now = Rtc.GetDateTime(); //Pega a data e horário atual

  Serial.println();

    if (!now.IsValid()) //Testa o horário e verifica se é válido
      Serial.println("Verificar a bateria ou o módulo pois a hora não é válida");

    int dia, mes, ano;
    int hora, minuto, segundo;

    dia = now.Day();
    mes = now.Month();
    ano = now.Year();

    hora = now.Hour();
    minuto = now.Minute();
    segundo = now.Second();

    sprintf(data, "%02u/%02u/%04u", dia, mes, ano);
    sprintf(horario, "%02u:%02u:%02u", hora, minuto, segundo);

    lcd.clear();

    lcd.setCursor(3,0);
    lcd.print(data);

    lcd.setCursor(4,1);
    lcd.print(horario);

    if(mfrc522.PICC_IsNewCardPresent()) //Verifica se há cartão no leitor
    {
      unsigned long uid = getID(); //Puxa a função que pega o UID

        if(uid != -1) //Já que retornou valor diferente de 1, significa que tem um cartão válido no leitor
        {
          Serial.print("Card detected, UID: "); 
          Serial.println(uid); //Imprime a UID do cartão

,          myFile.println("%lu;%02u/%02u/%04u;%02u:%02u:%02u", uid, dia, mes, ano, hora, minuto, segundo);  
          Serial.println();
        }
    }

    delay(1000);
}
