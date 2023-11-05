// (Heavily) adapted from https://github.com/G6EJD/ESP32-8266-Audio-Spectrum-Display/blob/master/ESP32_Spectrum_Display_02.ino
// Adjusted to allow brightness changes on press+hold, Auto-cycle for 3 button presses within 2 seconds
// Edited to add Neomatrix support for easier compatibility with different layouts.


//-----------LED-STRip-------
#include "FastLED.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define LED_PIN    6
#define LED_COUNT  56
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

//-------RENDER----------
float sensitivity = 1;//{ 2.1, 1.9, 1.7, 1.3, 1.2, 1.1, 1, 1.1};
uint32_t colorPerBand[] = {strip.Color(171, 0, 255), strip.Color(0,34,255), strip.Color(0,247,255), strip.Color(0,255,77),
                           strip.Color(222,255,0), strip.Color(255,171,0), strip.Color(255,26,0), strip.Color(255,0,137)} ;
#define AMPLITUDE       150  //250 //1000        // Depending on your audio source level, you may need to alter this value. Can be used as a 'sensitivity' control.
#define NUM_BANDS       7           // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands

const uint8_t kMatrixWidth = 7;                          // Matrix width
const uint8_t kMatrixHeight = 8;   
#define TOP  9 //            (kMatrixHeight - 0)   // Matrix height
// Sampling and FFT stuff
byte peak[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};              // The length of these arrays must be >= NUM_BANDS
int oldBarHeights[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int bandValues[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//-------BUTTON-------------
#define BUTTON_PIN   7
bool buttonPressed_thisLoop = false;
bool lastButtonState = false;

//--------MSGEQ7-----------------
int analogPin = 0; // read from multiplexer using analog input 0
int strobePin = 4; // strobe is attached to digital pin 4              WAS INITIALLY 2
int resetPin = 2; // reset is attached to digital pin 3
int spectrumValue[7]; // to hold a2d values

//-----------Rotary-Encoder-----
#define CLK 9
#define DT 8
#define SW 3
int currentStateCLK;
int lastStateCLK;
int RotaryPosChange = 0;
bool Rotary_button_Pressed = false;

//---------STATE_LOGIC-------
int curState = 0;
#define lastState  2
#define analyser_state  0

#define naturalLigh_state 1
double whitePercent = 0;

#define solidColor_state 2
int hue = 0;
#define maxHue 65535

//-------------debug-------------
String output = "";
String outpIntensStr;


void setup() {
//-----Debug-----
  //Serial.begin(9600);
  
//-----LED-STRIP---------
  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255) //100
//-----MSGEQ7-------------
  pinMode(analogPin, INPUT); //MSGEQ7 shit
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  analogReference(DEFAULT );
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);
//-----Button------------
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, HIGH);
//-----Rotary-Encoder-----
  pinMode(CLK, INPUT);   // Set encoder pins as inputs
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  lastStateCLK = digitalRead(CLK);  // Read the initial state of CLK 
}


bool buttonPressedThisFrame(){
  bool result = false;
  if(digitalRead(BUTTON_PIN) == LOW and lastButtonState == false){
    result = true;
  }
  
  lastButtonState = (digitalRead(BUTTON_PIN) == LOW);
  return result;
  }

void getRotaryChanges(){
  currentStateCLK = digitalRead(CLK);  // Read the current state of CLK
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
    if (digitalRead(DT) != currentStateCLK) {
      RotaryPosChange = 1;;
    } else {
      RotaryPosChange = -1;;
    }
  }
  else{ RotaryPosChange = 0;}
  lastStateCLK = currentStateCLK;

  Rotary_button_Pressed = (digitalRead(SW) == LOW);
  delay(1);
  }




void loop() {
  buttonPressed_thisLoop = buttonPressedThisFrame();
  getRotaryChanges();
  
  if (buttonPressed_thisLoop){
      curState ++;
      if (curState > lastState)curState = 0; 
  }

  if (RotaryPosChange != 0){
    if (Rotary_button_Pressed == true){changeBrightness(RotaryPosChange);}
    else{
      optionsChange(RotaryPosChange);
      }
    }

   renderState();
}

void changeBrightness(int change){
  uint8_t brightness = strip.getBrightness();
  int brightness_change = int( change * 10);
  brightness += brightness_change;
  if (brightness < 10) brightness = 10;
  if (brightness > 100) brightness = 100;
  strip.setBrightness(brightness);
  }

void optionsChange(int change){
  switch(curState){
    case analyser_state:
      changeSensitivity(change);
    break;
    
    case naturalLigh_state:
      changeNatural(change);      
    break;
    
    case solidColor_state:
      changeHue(change);
    break;    
    }
  }

void changeSensitivity(int change){
  sensitivity += change * 0.1;
  if (sensitivity < 0.1) sensitivity = 0.1;
  if (sensitivity > 20) sensitivity = 20;
  }

void changeNatural(int change){
  whitePercent += change * 0.05;
  if (whitePercent < 0) whitePercent = 0;
  if (whitePercent > 1) whitePercent = 1;
  
  }

void changeHue(int change) {
  hue += change * maxHue/40;
  if (hue < 0) hue = maxHue + hue;
  if (hue > maxHue) hue = hue - maxHue;
  }

void renderState(){
  switch(curState){
    case analyser_state:
      renderSpectrum();
    break;
    
    case naturalLigh_state:
      renderNatural();
    break;
    
    case solidColor_state:
      renderColor();
    break;    
    }
  }

void renderColor(){
  strip.fill(strip.gamma32(strip.ColorHSV(hue)));
  strip.show();
  }

void renderNatural(){
  int r = int (255 * whitePercent);
  int g = int (255 * whitePercent);
  int b = int (255 * whitePercent);
  int w = 255;
  strip.fill(strip.gamma32(strip.Color(r,g,b,w)));
  strip.show();
  }

void renderSpectrum(){
  // Reset bandValues[]
  for (int i = 0; i<NUM_BANDS; i++){
    bandValues[i] = 0;
  }
  strip.clear();
  output = String();

  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);

  for (int i = 0; i < 7; i++)
  {
   digitalWrite(strobePin, LOW);
   delayMicroseconds(35); // to allow the output to settle
   spectrumValue[i] = analogRead(analogPin);
  
   digitalWrite(strobePin, HIGH);
  }
  
  
  // Process the data into bar heights
  //output = "";  //Debug
  for (byte band = 0; band < NUM_BANDS; band++) {

    // Scale the bars for the display
    int barHeight = (int) spectrumValue[band] * 9 * sensitivity;
    barHeight = (int) barHeight/ 1021;//AMPLITUDE
    if (barHeight > TOP) barHeight = TOP;

    // Small amount of averaging between frames
    barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;

    // Move peak up
    if (barHeight > peak[band]) {
      peak[band] = min(TOP, barHeight);
    }
      for(int i = 0; i < 8 ; i++) {
        int pos = i + (8 * band);
        if(i <= barHeight){
          if(band % 2 == 0)
            pos = i + (8 * (band+1)) - (i * 2) - 1;
          uint32_t color = colorPerBand[band];
          strip.setPixelColor(pos, strip.gamma32(color));
      }
      }

    /*
      outpIntensStr = "";  // Debug
      for(int y = 0 ; y < barHeight ; y++){  //debug
        outpIntensStr += "#";                 //debug
      }
      output += toConstantLength(outpIntensStr, 8) + ".  ";  */ //debug

  // Save oldBarHeights for averaging later
  oldBarHeights[band] = barHeight;
  }
  //Serial.print( output + "\n");               //debug
  strip.show();
 }

String toConstantLength(String in, int goalLen){
  if (in.length()>= goalLen ) return in;
  while (in.length() < goalLen){
    in += " ";
  }
  return in;
}
