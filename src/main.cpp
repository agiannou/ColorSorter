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

void solenoid (void* p_params)
{
  (void)p_params;            // Does nothing but shut up a compiler warning

}

void steppermotor (void* p_params)
{
  (void)p_params;            // Does nothing but shut up a compiler warning

    const TickType_t sim_period = 10;         // RTOS ticks (ms) between runs

    // Initialise the xLastWakeTime variable with the current time.
    // It will be used to run the task at precise intervals
    TickType_t xLastWakeTime = xTaskGetTickCount();
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
