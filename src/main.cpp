/** @file main.cpp
 *    This is the main file for the color sorter project for ME 507.
 * 
 * @author Alexander Giannousis, Ali Yazdkhasti, Raul Gonzalez
 * @date   2020-Nov-16 Created file
 */


#include <Arduino.h>
#include <PrintStream.h>
#include <Stepper.h>
#include "Adafruit_TCS34725.h"

/** Ain pins are:
 *  D6=>PWM_A (PB10)
 *  D3=>Ain2  (PB3)
 *  D13=>Ain1 (PA5)
 *  Bin pins are:
 *  D2=>PWM_B (PA10)
 *  D8=>Bin2  (PA9)
 *  D7=>Bin1  (PA8)
 */
// inputs for coil A on stepper motor 
const int8_t PWMA = PB10;
const int8_t Ain2 = PB3;
const int8_t Ain1 = PA5;

// inputs for coil B on stepper motor
const int8_t PWMB = PA10;
const int8_t Bin2 = PA9;
const int8_t Bin1 = PA8;

//  set PWMA and PWMB to VCC 

const int8_t STEPS_PER_REV = 200;

Stepper myStepper(STEPS_PER_REV,Ain2,Ain1,Bin1,Bin2);


void setup()
{
      Serial.begin (115200)
      Serial << endl <<"Stepper Test" << endl;
      stepper.setSpeed(60);   
}

void loop() 
{
  // put your main code here, to run repeatedly:
}
