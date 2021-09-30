#include <ColorConverterLib.h>
#include <math.h>

#define R_pin 16
#define G_pin 17
#define B_pin 5

const int freq = 5000;
const int R_ledChannel = 0;
const int G_ledChannel = 1;
const int B_ledChannel = 2;
const int resolution = 8; //number of bits

void setup() {
  ledcSetup(R_ledChannel,freq,resolution);
  ledcSetup(G_ledChannel,freq,resolution);
  ledcSetup(B_ledChannel,freq,resolution);

  ledcAttachPin(R_pin,R_ledChannel);
  ledcAttachPin(B_pin,B_ledChannel);
  ledcAttachPin(G_pin,G_ledChannel);

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

  ColorConverter::HsvToRgb(hue,saturation,lighting,red,green,blue);  
  ledcWrite(R_ledChannel,red);
  ledcWrite(G_ledChannel,green);
  ledcWrite(B_ledChannel,blue); 
  delay(10);


}
