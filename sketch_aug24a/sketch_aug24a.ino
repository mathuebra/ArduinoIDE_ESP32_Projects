#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx1 = Adafruit_MLX90614();

void setup() {
  mlx1.begin();
}

void loop() {
  
  float Object_Temperature1 = mlx1.readAmbientTempC();
  
  printf("%f\r\n", Object_Temperature1); 
}
