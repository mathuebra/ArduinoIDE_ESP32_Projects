#include <LiquidCrystal_I2C.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);

void setup() {
  lcd.begin(0x3F, 16, 2);
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Hello World");

}

void loop() {

  
}
