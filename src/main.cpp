/** @file main.cpp
 *    This is the main file for the color sorter project for ME 507.
 * 
 * @author Alexander Giannousis, Ali Yazdkhasti, Raul Gonzalez
 * @date   2020-Nov-16 Created file
 * @date   2020-Nov-18 Added tasks
 * @date   2020-Nov-27 Tried fixing solenoid task
 * @date   2020-Nov-28 Finally fixed solenoid task
 * @date   2020-Nov-29 
 */

#include <Arduino.h>
#include <PrintStream.h>
#if (defined STM32L4xx || defined STM32F4xx)
    #include <STM32FreeRTOS.h>
#endif
#include <Stepper.h>
#include "Adafruit_TCS34725.h"
#include "taskshare.h"

// share for communicating between solenoid/stepper tasks
Share<bool> solenoid_on ("Solenoid on/off switch");

// example of what is still needed
/** @brief   Read an integer from a serial device, echoing input and blocking.
 *  @details This function reads an integer which is typed by a user into a
 *           serial device. It uses the Arduino function @c readBytes(), which
 *           blocks the task which calls this function until a character is
 *           read. When any character is received, it is echoed through the
 *           serial port so the user can see what was typed. Only decimal
 *           integers are supported; negative integers beginning with a single
 *           @c - sign or positive ones with a @c + will work. 
 * 
 *           @b NOTE: The serial device must have its timeout set to a very
 *           long time, or this function will malfunction. A recommended call:
 *           @code
 *           Serial.setTimeout (0xFFFFFFFF);
 *           @endcode
 *           Assuming that the serial port named @c Serial is being used.
 *  @param   stream The serial device such as @c Serial used to communicate
 */


/** Ain pins are:
 *  D6=>PWM_A (PB10)
 *  D3=>Ain2 (PB3)
 *  D13=>Ain1 (PA5)
 *  Bin pins are:
 *  D2=>PWM_B (PA10)
 *  D8=>Bin2  (PA9)
 *  D7=>Bin1  (PA8)
 *  D10=>Ain_sol (PB6)
 */
// inputs for coil A on stepper motor 
const int8_t PWMA = PB10;
const int8_t Ain2 = PB3;
const int8_t Ain1 = PA5;

// inputs for coil B on stepper motor
const int8_t PWMB = PA10;
const int8_t Bin2 = PA9;
const int8_t Bin1 = PA8;

/** TB6612FNG motor driver input pins
 *  PWMA=>D12()
 *  AIN_2=>D11()
 *  AIN_1=>D10()
 *  BIN_1=>D09()
 *  BIN_2=>D5()
 *  PWMB=>D4()
 */
const int8_t Ain_sol = PB6;



// setting the up the number of stepper motor steps
#define STEPS_PER_REV 200

// setting up the input pins for the motor driver
Stepper myStepper(STEPS_PER_REV,Ain2,Ain1,Bin1,Bin2);

void steppermotor (void* p_params)
{
    Serial << "Now initializing stepper task" << endl;
    (void)p_params;            // Does nothing but shut up a compiler warning
    myStepper.setSpeed(60);

    //  set PWMA and PWMB to VCC 
    digitalWrite(PWMA,HIGH);
    digitalWrite(PWMB,HIGH);

    const TickType_t stepper_period = 500;         // RTOS ticks (ms) between runs

    // Initialise the xLastWakeTime variable with the current time.
    // It will be used to run the task at precise intervals
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // init variable to pull from share and init share as off
    bool sol_on_1;
    solenoid_on.put(false);

    for(;;)
    {
      Serial << "Now entering stepper task loop" << endl;
      // check solenoid share
      solenoid_on.get(sol_on_1);
      // if share is off, drive stepper motor by designated number of steps and set share to on.
      if (sol_on_1== false)
      {
        Serial << "Now turning on stepper motor" << endl;
        myStepper.step(STEPS_PER_REV);
        delay(5000);
        solenoid_on.put(true);
      }
      else
      {
        Serial << "stepper motor is off" << endl;
      }
      
        // Delay task by 10 ms since task began
        vTaskDelayUntil (&xLastWakeTime, stepper_period);
    }
}


void solenoid (void* p_params)
 {
   Serial << "Now initializing solenoid task" << endl;
   (void)p_params;            // Does nothing but shut up a compiler warning
  
     const TickType_t solenoid_period = 2500;         // RTOS ticks (ms) between runs

     // Initialise the xLastWakeTime variable with the current time.
     // It will be used to run the task at precise intervals
     TickType_t xLastWakeTime = xTaskGetTickCount();

     // init variable to pull from share
     bool sol_on;
     for(;;)
     {
         Serial << "Now entering solenoid task loop" << endl;
         // check solenoid share
         solenoid_on.get(sol_on);
         
         // if share is on, turn on solenoid, leave on for 1 sec, then turn off
         if (sol_on == 1)
         {
           Serial << "Now turning on solenoid" << endl;
           digitalWrite(Ain_sol, HIGH);
           delay(5000);
           solenoid_on.put(false);
         }
         // if share is off, turn off solenoid
         else
         {
           Serial << "Solenoid off" << endl;
           digitalWrite(Ain_sol, LOW);
         }
             // Delay task by 50 ms since task began
         vTaskDelayUntil (&xLastWakeTime, solenoid_period);
    }
 }

void setup() {
// Start the serial port, wait a short time, then say hello. Use the
    // non-RTOS delay() function because the RTOS hasn't been started yet
    Serial.begin (115200);
    delay (5000);
    Serial << endl << endl << "Starting Color Sorter Demonstration Program" << endl;

    //creating the accelerometer reading task
     xTaskCreate (solenoid,
                 "Run solenoid",                     // Name for printouts
                 1024,                            // Stack size
                 (void*)(&Ain_sol),                            // Parameters for task fn.
                 10,                               // Priority
                 NULL);                           // Task handle
    //creating the acceleration printing task
     xTaskCreate (steppermotor,
                 "Run stepper motor",                  // Name for printouts
                 2048,                            // Stack size
                 (void*)(&PWMA, &PWMB, &Ain1, &Ain2, &Bin1, &Bin2), // Parameters for task fn.
                 5,                               // Priority
                 NULL);                           // Task handle



    // If using an STM32, we need to call the scheduler startup function now;
    // if using an ESP32, it has already been called for us
    #if (defined STM32L4xx || defined STM32F4xx)
        vTaskStartScheduler ();
    #endif
}

void loop()
{
}
