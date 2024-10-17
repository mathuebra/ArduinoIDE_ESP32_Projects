#include <SparkFunMLX90614.h>
#include <Wire.h>
//#include <Adafruit_MLX90614.h>

//Adafruit_MLX90614 mlx = Adafruit_MLX90614();

IRTherm topSensor, midSensor, botSensor;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Wire.begin();
  
////  // Tenta conectar nos sensores
//  if(!topSensor.begin())
//  {
//    Serial.println("Falhou em comunicar com o sensor 0x5A");
//    while (1);
//  }
   
//  // Tenta conectar nos sensores
//  if(!midSensor.begin(0x5B))
//  {
//    Serial.println("Falhou em comunicar com o sensor 0x5B");
//    while (1);
//  }
  
  // Tenta conectar nos sensores
  if(!botSensor.begin(0x5B))
  {
    Serial.println("Falhou em comunicar com o sensor 0x5C");
    while (1);
  }

//  botSensor.setAddress(0x5A);
  topSensor.setUnit(TEMP_C);
  midSensor.setUnit(TEMP_C);
  botSensor.setUnit(TEMP_C);
  
}

void loop() {
//
  if(!topSensor.read())
  {
    Serial.println("Nao Conseguiu Ler do 0x5A");
  }
  if(!midSensor.read())
  {
    Serial.println("Nao Conseguiu Ler do 0x5B");
  }
  if(!botSensor.read())
  {
    Serial.println("Nao Conseguiu Ler do 0x5C");
  }
  
  
  Serial.println("Top Sensor (Object/Ambient): " + String(topSensor.object()) + " °C " + String(topSensor.ambient()) + " °C ");

  Serial.println("Mid Sensor (Object/Ambient): " + String(midSensor.object()) + " °C " + String(midSensor.ambient()) + " °C ");

  Serial.println("Bot Sensor (Object/Ambient): " + String(botSensor.object()) + " °C " + String(botSensor.ambient()) + " °C ");
  
  delay(300);
}
