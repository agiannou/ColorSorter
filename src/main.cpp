/** @file main.cpp
 *  @brief   This is the main file for the color sorter project for ME 507.
 *           The objective of this function is to detect the color balls on a turn table 
 *           and sort them out in 3 separate piles. 
 *  @details This functions reads the color of a ball on the a turn table and 
 *           sends the color of the ball back to the Nucleo board using the SDA and 
 *           SCL pins. Once the color of the ball is recived the motor is then sent a 
 *           signal to turn to the correct postion based on the color of the ball.
 *           To make things easier, the turn table was split into 4 sections such that the 
 *           motor will only be moving in 90 degree increments in order to get to the correct 
 *           location.
 * 
 * @author Alexander Giannousis, Ali Yazdkhasti, Raul Gonzalez
 * @date   2020-Nov-16 Created file
 * @date   2020-Nov-18 Added tasks
 * @date   2020-Nov-27 Tried fixing solenoid task
 * @date   2020-Nov-28 Finally fixed solenoid task
 * @date   2020-Nov-29 Added Color Sensor task
 */
//
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

// Create an object for the color sensor class
Adafruit_TCS34725 my_ColorSensor;



/** @brief   Function used to control the stepper motor
 *  @details The input for the stepper motor will come from the color sensor. Depending 
 *           on which color is registered the stepper motor will turn until it has 
 *           reached the correct spot and stop
 *           
 *          
 *  @param   PWMA the input pin to the H-bridge chip for pulse modulation for the A side 
 *  @param   PWMB the input pin to the H-bridge chip for pulse modulation for the A side 
 *           Because the we are using a stepper motor we do not need the PWM signal pins 
 *           so they can be set to high
 *  @param   Ain2 input pin for the A side of the motor driver
 *  @param   Ain1 input pin for the A side of the motor driver
 *  @param   Bin1 input pin for the B side of the motor driver
 *  @param   Bin2 input pin for the B side of the motor driver
 * 
 */
void steppermotor (void* p_params)
{
    (void)p_params;            // Does nothing but shut up a compiler warning
    myStepper.setSpeed(60);
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


    // setting the up the number of stepper motor steps
    #define STEPS_PER_REV 200

    // setting up the input pins for the motor driver
    Stepper myStepper(STEPS_PER_REV,Ain2,Ain1,Bin1,Bin2);
    //  set PWMA and PWMB to VCC 
    digitalWrite(PWMA,HIGH);
    digitalWrite(PWMB,HIGH);

    const TickType_t stepper_period = 10;         // RTOS ticks (ms) between runs

    // Initialise the xLastWakeTime variable with the current time.
    // It will be used to run the task at precise intervals
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // init variable to pull from share and init share as off
    bool sol_on_1;
    solenoid_on.put(false);

    for(;;)
    {
      // check solenoid share
      solenoid_on.get(sol_on_1);
      // if share is off, drive stepper motor by designated number of steps and set share to on.
      if (sol_on_1== false)
      {
        myStepper.step(STEPS_PER_REV);
        delay(1000);
        solenoid_on.put(true);
      }
      else
      {
      }
      
        // Delay task by 10 ms since task began
        vTaskDelayUntil (&xLastWakeTime, stepper_period);
    }
}


/** @brief   This code controls the solenoids to open and close
 *  @details The signal to open comes depending on what color the color sensor has 
 *           registered. The solenoid will wait until the stepper motor has turned 
 *           to the correct location before opening up. (*NOTE: do not keep solenoid 
 *           on for prolonged peroids of time this will burn it out)
 *          
 *  @param   PWMA the input pin to the H-bridge chip for pulse modulation for the A side 
 *  @param   PWMB the input pin to the H-bridge chip for pulse modulation for the A side 
 *           Because the we are using not using a DC motor we have no use for this, but 
 *           the according to the datasheet they must be set to high.
 *  @param   Ain2 controls the 1st solenoid 
 *  @param   Ain1 controls the 2nd solenoid
 *  @param   Bin1 controls the 3rd solenoid
 *  @param   Bin2 controls the 4th solenoid
 * 
 */
void solenoid (void* p_params)
 {
   (void)p_params;            // Does nothing but shut up a compiler warning
  
     const TickType_t solenoid_period = 50;         // RTOS ticks (ms) between runs
     /** TB6612FNG motor driver input pins
       *  PWMA=>D12()
       *  AIN_2=>D11()
       *  AIN_1=>D10()
       *  BIN_1=>D09()
       *  BIN_2=>D5()
       *  PWMB=>D4()
      */
    const int8_t Ain_sol = PB6;
      
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
         if (sol_on == 1)
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

/** @brief   This function reads the color sensor 
 *  @details This function reads the color sensor and send a signal 
 *           to the stepper motor to turn until it has reached the 
 *           correct location.            
 *          
 *  @param   r used to store a value of the red detected 
 *  @param   g used to store a value of the green detected 
 *  @param   b used to store a value of the blue detected 
 * 
 */
void ColorSensor (void* p_params)
{
  (void)p_params;            // Does nothing but shut up a compiler warning
  // init variables for reading rgb colors
  float r;
  float g;
  float b;

  for(;;)
  {
    // get color and print individual RGB values
    my_ColorSensor.getRGB(&r, &g, &b);
    Serial << "R: " << r << endl << "G: " << g << "B: " << endl;
    delay(500);
  }
}


void setup() {
// Start the serial port, wait a short time, then say hello. Use the
    // non-RTOS delay() function because the RTOS hasn't been started yet
    Serial.begin (115200);
    delay (5000);
    Serial << endl << endl << "Starting Color Sorter Demonstration Program" << endl;

    //creating the solenoid task
     xTaskCreate (solenoid,
                 "Run solenoid",                     // Name for printouts
                 1024,                            // Stack size
                 (void*)(&Ain_sol),                            // Parameters for task fn.
                 10,                               // Priority
                 NULL);                           // Task handle
    //creating the stepper motor task
     xTaskCreate (steppermotor,
                 "Run stepper motor",                  // Name for printouts
                 2048,                            // Stack size
                 (void*)(&PWMA, &PWMB, &Ain1, &Ain2, &Bin1, &Bin2), // Parameters for task fn.
                 5,                               // Priority
                 NULL);                           // Task handle
    //creating the color sensor task
     xTaskCreate (ColorSensor,
                 "Get data from color sensor",     // Name for printouts
                 1024,                            // Stack size
                 NULL,                            // Parameters for task fn.
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
