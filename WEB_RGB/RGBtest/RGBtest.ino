#include <ColorConverterLib.h>
#include <math.h>

#define R_pin 11
#define G_pin 9
#define B_pin 10

void setup() {
  // put your setup code here, to run once:
  pinMode(R_pin, OUTPUT);
  pinMode(G_pin, OUTPUT);
  pinMode(B_pin, OUTPUT);

  Serial.begin(9600);
}

int i = 0;
uint8_t red, green, blue = 0;
double saturation, lighting, hue;


void loop() {
  if(i<255){
    i++;
  } else{
    i = 0;
  }
  lighting = 0.5;
  saturation = 1;
  hue = (double) i/255;
 
  Serial.print("hue: ");
  Serial.print(hue);
  ColorConverter::HsvToRgb(hue,saturation,lighting,red,green,blue);   
  Serial.print(" red: ");
  Serial.print(red);
  Serial.print(" blue: ");
  Serial.print(blue);
  Serial.print(" green: ");
  Serial.println(green);
  analogWrite(R_pin, red);
  analogWrite(G_pin, green);
  analogWrite(B_pin, blue);


  delay(10);
}
