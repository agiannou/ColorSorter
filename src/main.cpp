/** @file main.cpp
 *    This is the main file for the color sorter project for ME 507.
 * 
 * @author Alexander Giannousis, Ali Yazdkhasti, Raul Gonzalez
 * @date   2020-Nov-16 Created file
 * @date   2020-Nov-18 Added tasks
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
Share<bool> solenoid_on ("Power");

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

// solenoid pins
const int8_t Ain_sol = PB6;

//  set PWMA and PWMB to VCC 


// setting the up the stepper number of stepper motor steps
const int8_t STEPS_PER_REV = 200;

// setting up the input pins for the motor driver
Stepper myStepper(STEPS_PER_REV,Ain2,Ain1,Bin1,Bin2);

void solenoid (void* p_params)
{
  (void)p_params;            // Does nothing but shut up a compiler warning
  
    const TickType_t solenoid_period = 50;         // RTOS ticks (ms) between runs

    // Initialise the xLastWakeTime variable with the current time.
    // It will be used to run the task at precise intervals
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // init variable to pull from share
    bool sol_on;

    for(;;)
    {
        // check solenoid share
        solenoid_on.get(sol_on);
        // if share is on, turn on solenoid, leave on for 1 sec, then turn off
        if (sol_on == true)
        {
          digitalWrite(Ain_sol, HIGH);
          delay(1000);
          solenoid_on.put(false);
        }
        // if share is off, turn off solenoid
        else
        {
          digitalWrite(Ain_sol, LOW);
        }
            // Delay task by 50 ms since task began
        vTaskDelayUntil (&xLastWakeTime, solenoid_period);
    }
}

void steppermotor (void* p_params)
{
  (void)p_params;            // Does nothing but shut up a compiler warning
    myStepper.setSpeed(60);

    //  set PWMA and PWMB to VCC 
    digitalWrite(PWMA,HIGH);
    digitalWrite(PWMB,HIGH);

    const TickType_t stepper_period = 10;         // RTOS ticks (ms) between runs

    // Initialise the xLastWakeTime variable with the current time.
    // It will be used to run the task at precise intervals
    TickType_t xLastWakeTime = xTaskGetTickCount();

    //init variable to pull from share
    bool sol_on;

    for(;;)
    {
      // check solenoid share
      solenoid_on.get(sol_on);
      // if share is off, drive stepper motor by designated number of steps and set share to on.
      if (sol_on == false)
      {
        myStepper.step(STEPS_PER_REV);
        solenoid_on.put(true);
      }
            // Delay task by 10 ms since task began
        vTaskDelayUntil (&xLastWakeTime, stepper_period);
    }
}

void setup() {
// Start the serial port, wait a short time, then say hello. Use the
    // non-RTOS delay() function because the RTOS hasn't been started yet
    Serial.begin (115200);
    delay (5000);
    Serial << endl << endl << "Starting Color Sorter Demonstration Program" << endl;

    // Create a task ffor solenoid
    xTaskCreate (solenoid,
                 "Solenoid task",                     // Name for printouts
                 1024,                            // Stack size
                 NULL,                            // Parameters for task fn.
                 4,                               // Priority
                 NULL);                           // Task handle

    // Create a task which prints a more agreeable message
    xTaskCreate (steppermotor,
                 "Stepper motor task",
                 1024,                            // Stack size
                 NULL,
                 2,                               // Priority
                 NULL);

    // If using an STM32, we need to call the scheduler startup function now;
    // if using an ESP32, it has already been called for us
    #if (defined STM32L4xx || defined STM32F4xx)
        vTaskStartScheduler ();
    #endif
}

void loop() {
}
