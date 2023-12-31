#include "Common.h"
#include "LightStrip.h"
#include "BottomLights.h"
#include "LightPanel.h";

#include "FastLED.h"
#include "Wire.h"

#define I2CADDRESS 0x20  // 25 in Hexadecmal

char commandBuffer[MAX_COMMAND_LENGTH];
String commandString = "";
volatile boolean commandComplete = false;
unsigned long ledtimer = 0;

CRGB frontleds[NUM_FRONT_LEDS];
CRGB leftleds[NUM_LEFT_LEDS];
CRGB rightleds[NUM_RIGHT_LEDS];

LightPanel *frontLights;
LightPanel *leftLights;
LightPanel *rightLights;
LightStrip *topLights;
BottomLights *bottomLights;

#define NUM_ANIMATORS 5

IAnimatorBase *animators[NUM_ANIMATORS];

void setup() {
  Serial.begin(9600);
  commandString.reserve(MAX_COMMAND_LENGTH);

  Wire.begin(I2CADDRESS);
  Wire.onReceive(i2cEvent);

  frontLights = LightPanel::addLightPanel<FRONT_PIN>(frontleds, NUM_FRONT_LEDS, DEFAULT_FRONT_COLOR);
  leftLights = LightPanel::addLightPanel<LEFT_PIN>(leftleds, NUM_LEFT_LEDS, DEFAULT_LEFT_COLOR);
  rightLights = LightPanel::addLightPanel<RIGHT_PIN>(rightleds, NUM_RIGHT_LEDS, DEFAULT_RIGHT_COLOR);
  topLights = LightStrip::addLightStrip<TOP_LIGHTS_PIN>(NUM_TOP_LEDS, DEFAULT_TOP_COLOR);
  bottomLights = BottomLights::initialize<BOTTOM_LIGHTS_PIN>();

  animators[0] = frontLights;
  animators[1] = leftLights;
  animators[2] = rightLights;
  animators[3] = topLights;
  animators[4] = bottomLights;

  Serial.println("Ready");
}

void processSequence(char sequence) {
  switch(sequence) {
    case 0:
    Serial.println("caso 0");
      frontLights->processCommand("F18", 3);
      leftLights->processCommand("L10", 3);
      rightLights->processCommand("R10", 3);
      topLights->processCommand("T245", 4);
      bottomLights->processCommand("B10", 3);
      break;
  }
}

void loop() {

  /////prueba para mandar datos a arduino///////////


  
  //////////////////////////////////////////////////
  unsigned long currentTime = millis();
  
  if(commandComplete) {
    commandString.trim();

    Serial.print(F("Received Command: "));
    Serial.println(commandString);

    int commandLength = commandString.length();
    commandString.toCharArray(commandBuffer, MAX_COMMAND_LENGTH);
    commandString="";
    commandComplete = false;

    switch(commandBuffer[0]) {
      case 'T':
        topLights->processCommand(commandBuffer, commandLength);
        break;
      
      case 'W':
        leftLights->processCommand(commandBuffer, commandLength);
        rightLights->processCommand(commandBuffer, commandLength);
        break;
      
      case 'L':
        leftLights->processCommand(commandBuffer, commandLength);
        break;
      
      case 'R':
        rightLights->processCommand(commandBuffer, commandLength);
        break;

      case 'F':
        frontLights->processCommand(commandBuffer, commandLength);
        break;

      case 'B':
        bottomLights->processCommand(commandBuffer, commandLength);
        break;

      case 'X':
        for (int i = 0; i < NUM_ANIMATORS; i++) {
          animators[i]->stop();
        }
        break;

      case 'S':
        if (commandLength > 1 && isDigit(commandBuffer[1])) {
          processSequence(commandBuffer[1] - '0');
        }
        break;
    }
  }

  for (int i = 0; i < NUM_ANIMATORS; i++) {
    animators[i]->update(currentTime);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////                                                                                               /////
/////                             Serial & I2C Communication Functions                              /////
/////                                                                                               /////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
           
/////////////////////////////////////////////////////////
///*****          Serial Event Function          *****///
/////////////////////////////////////////////////////////
///Esta rutina se ejecuta entre ejecuciones de bucle(),//
/// por lo que usar el retraso dentro del bucle       ///
/// puede retrasar la respuesta.                      ///
/// múltiple bytes de datos pueden estar disponibles. ///
/////////////////////////////////////////////////////////
      
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\r' || inChar == '\n') {
      commandComplete = true;
    } else {
      commandString += inChar;
    }
  }
}

/////////////////////////////////////////////////////////
///*****            I2C Event Function           *****///
/////////////////////////////////////////////////////////
///Esta rutina se ejecuta cuando se produce un evento ///
/// onRecieve motivado. Múltiples bytes de datos      ///
///             pueden ser disponible.                ///
/////////////////////////////////////////////////////////            
      
void i2cEvent(int howMany)
{  
  commandString = "";
  int i=0;
  while(Wire.available()) {
    char inChar = (char)Wire.read();
    if(i<MAX_COMMAND_LENGTH) {
       commandString += inChar;
    }
    
    i++;
  }
  commandComplete = true;
}
