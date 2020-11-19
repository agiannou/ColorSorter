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


void solenoid (void* p_params)
{
  (void)p_params;            // Does nothing but shut up a compiler warning
}

void steppermotor (void* p_params)
{
  (void)p_params;            // Does nothing but shut up a compiler warning
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
                 1,                               // Priority
                 NULL);                           // Task handle

    // Create a task which prints a more agreeable message
    xTaskCreate (steppermotor,
                 "Stepper motor task",
                 1024,                            // Stack size
                 NULL,
                 5,                               // Priority
                 NULL);

    // If using an STM32, we need to call the scheduler startup function now;
    // if using an ESP32, it has already been called for us
    #if (defined STM32L4xx || defined STM32F4xx)
        vTaskStartScheduler ();
    #endif
}

void loop() {
}
