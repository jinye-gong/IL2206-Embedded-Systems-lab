/**
 * @file main.c
 * @author Ingo Sander (ingo@kth.se)
 * @brief Skeleton for cruise control application
 *        The skeleton code runs on the ES-Lab-Kit, 
 *        has very limited functionality and needs to be
 *        modified.
 *          
 * @version 0.1
 * @date 2025-09-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "bsp.h"
#include "hardware/clocks.h"
#include "timers.h"

#define GAS_STEP 2  /* Defines how much the throttle is increased if GAS_STEP is asserted */

#define CRUISE_CONTROL  SW_7
#define GAS_PEDAL       SW_6
#define BRAKE_PEDAL     SW_5

/* Definition of handles for tasks */
TaskHandle_t    xButton_handle; /* Handle for the Button task */
TaskHandle_t    xControl_handle; /* Handle for the Control task */
TaskHandle_t    xVehicle_handle; /* Handle for the Vehicle task */
TaskHandle_t    xDisplay_handle; /* Handle for the Display task */
TaskHandle_t    xWatchdog_handle;
TaskHandle_t    xOverloadDetection_handle;
TaskHandle_t    xExtraLoad_handle;

/* Definition of handles for queues */
QueueHandle_t xQueueVelocity;
QueueHandle_t xQueuePosition;
QueueHandle_t xQueueThrottle;
QueueHandle_t xQueueCruiseControl;
QueueHandle_t xQueueGasPedal;
QueueHandle_t xQueueBrakePedal;
QueueHandle_t xQueueOverloadDetected;
QueueHandle_t xQueueOverloadState;
QueueHandle_t xQueueSwitches;

TimerHandle_t xWatchdogTimer;

// Cruise control FSM machine.
typedef enum {
    IDLE = 0,
    CRUISE_INIT = 1,
    CRUISE_ACTIVE = 2,
    CRUISE_EXIT = 3 
} STATE;

 /**
  * =======================================================================
  * vButtonTask(void *args):
  *     @brief The button task shall monitor the input buttons and send
  *            values of GAS, BRAKE, and CRUISE the other tasks.
  *     @param args corresponds to period of task (50ms).
  * 
  *     Function uses busy wait I/0 to monitor the buttons.
  *     Performs a single iteration of while loop that scans buttons,
  *     updates GAS, BRAKE, and CRUISE sets LEDS and ovewrites into queue.
  *     Delays until next period.
  */
void vButtonTask(void *args) {

    // TICK_RATE_HZ 1000 so technically args = 50 will become
    // 50 ms but it is safer to use pdMS_TO_TICKS(50);
    const TickType_t xPeriod = (uint32_t) args;

    // Needs current tik count to align with the period schedule.
    // After this assignment, xLastWakeTime is updated automatically 
    // internally within the vTaskDelayUntil() function.
    // TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t xLastWakeTime = 0;

    bool value_gas_pedal = true;
    bool value_brake_pedal = false;
    bool value_cruise_control = false;
    
    // holds flags for switches SW10-SW17 in order.
    uint8_t switch_pins = 0;

    /* Busy wait IO for Button input */
    /* Directly set LEDs from Button Task */
    /* Delay until next period */
    while(true) {

        // PULL_UP Buttons we need to take inverse.
        value_gas_pedal      = !BSP_GetInput(GAS_PEDAL);
        value_brake_pedal    = !BSP_GetInput(BRAKE_PEDAL);
        value_cruise_control = !BSP_GetInput(CRUISE_CONTROL);

        // Is this suppose to be done here directly? Or do we have 
        // To wait for command from controller to light them?
        BSP_SetLED(LED_GREEN,   value_gas_pedal);
        BSP_SetLED(LED_RED,     value_brake_pedal);

        // LEAVE CRUISE CONTROL LED COMPLETELY FOR 
        //BSP_SetLED(LED_YELLOW,  value_cruise_control);
        
        /* READ SWITCHES INPUT */
        switch_pins = 0;
        switch_pins = ( (BSP_GetInput(SW_10) << 7)
                      | (BSP_GetInput(SW_11) << 6)
                      | (BSP_GetInput(SW_12) << 5)
                      | (BSP_GetInput(SW_13) << 4)
                      | (BSP_GetInput(SW_14) << 3)
                      | (BSP_GetInput(SW_15) << 2)
                      | (BSP_GetInput(SW_16) << 1)
                      | (BSP_GetInput(SW_17)));

        xQueueOverwrite(xQueueGasPedal,     &value_gas_pedal);
        xQueueOverwrite(xQueueBrakePedal,   &value_brake_pedal);
        xQueueOverwrite(xQueueCruiseControl,&value_cruise_control);
        xQueueOverwrite(xQueueSwitches,     &switch_pins);

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}


void cruise_control_FSM(
    uint8_t *p_state, 
    bool *p_cruise_control_button, 
    bool gas_pedal, 
    bool brake_pedal, 
    uint16_t velocity,
    uint16_t *p_cruise_velocity,
    uint16_t *p_throttle
) {
    switch (*p_state)
    {
        case IDLE: {
            
            // YELLOW LED is off while cruise in inactive.
            BSP_SetLED(LED_YELLOW, 0);
            if(*p_cruise_control_button == 1)
                *p_state = CRUISE_INIT;
        } break;
        case CRUISE_INIT: {
            printf("CRUISE_STATE: INIT\n");
            // Wait for button to be unpressed. This state also sets the
            // desired cruise velocity held by the CRUISE_ACTIVE state.
            if(*p_cruise_control_button == 0) {
                *p_state = CRUISE_ACTIVE;
                *p_cruise_velocity = velocity;
            }
            printf("INIT: CRUISE V: %d\n", *p_cruise_velocity);
        } break;
        case CRUISE_ACTIVE: {
            printf("CRUISE_STATE: ACTIVE\n");

            // Yellow LED turned on while CRUISE is active.
            BSP_SetLED(LED_YELLOW, 1);

            // While in CRUISE if conditions no longer hold go directly to idle.
            if(gas_pedal || brake_pedal || (velocity < 25)) {
                *p_state = IDLE;
            }

            // If cruise button is pressed again go to cruise exit that works the 
            // same as cruiseinit (i.e. smooth button pressing)
            if(*p_cruise_control_button == 1) {
                *p_state = CRUISE_EXIT;
            }
            
            /* CRUISE CONTROL ALGORITHM */
            // Idea is based on this equation but throttle cannot be negative...
            // throttle += 8 * (cruise_velocity - velocity);
            // Retardation varies between approx. (-15, 17) 
            //so taking the average we get constant 8.
            // Holds +/- 4 (V) for lower velocities, as V goes higher (>70) it starts
            // to be more wavy amplified.

            printf("CRUISE V: %d, V: %d\n", *p_cruise_velocity, velocity);

            // Current V is above desired V.
            if(*p_cruise_velocity < velocity) {
                    *p_throttle = 0;
            }
            
            // Current V is below desired V
            if(*p_cruise_velocity > velocity) {
                *p_throttle += 8;
                if(*p_throttle > 80) {
                    *p_throttle = 80;
                }
            }
        } break;
        case CRUISE_EXIT: {
            printf("CRUISE_STATE: EXIT\n");
            if(*p_cruise_control_button == 0)
                *p_state = IDLE;
        } break;
        
        default: {
            *p_state = IDLE;
        } break;
    }
}

/**
 * =======================================================================
 * vControlTask(void *args):
 *      @brief The control tasks calculates the new throttle using your 
 *             control algorithm and the current values.
 *      @param args corresponds to period of task (200ms).
 * 
 *     ==> MODIFY THIS TASK!
 *     Currently the throttle has a fixed value of 80
 * 
 *      Read values of GAS, BRAKE, CRUISE, and VELOCITY to calculate 
 *      THROTTLE and send signal. 
 *      
 *      CRUISE ALGORITHM:
 *      CRUISE is true while GAS=0, BRAKE=0, VELOCITY>=25, 
 *      if this changes then update LED.
 *      When CRUISE is true velocity needs to be held (+-4 m/s) within
 *      cruise_velocity. 
 *      If velocity is less then increase, if less, decrease.
 */
void vControlTask(void *args) {

    const TickType_t xPeriod = (uint32_t) args;
    TickType_t xLastWakeTime = 0;

    //uint16_t throttle = 80;
    uint16_t throttle = 0;
    uint16_t velocity = 0;
    uint16_t cruise_velocity;

    //bool cruise_control_button;
    bool cruise_control;
    bool cruise_control_button;
    bool gas_pedal;
    bool brake_pedal;
    
    // initialize in state IDLE.
    uint8_t cruise_state = IDLE;

    // By aligning the else if statements in order of BRAKE, GAS, CRUISE
    // we automatically place operations in assigned priority.
    while(true) {

        cruise_control_FSM(
            &cruise_state, 
            &cruise_control_button, 
            gas_pedal, 
            brake_pedal, 
            velocity, 
            &cruise_velocity,
            &throttle);

        if (gas_pedal) {
            throttle += GAS_STEP; 
            if (throttle > 80) {
                throttle = 80;
            }
        }
        else 
        {
            // Case Nothing: (same as braking)
            // If we brake cruise_state goes to IDLE.
            if(cruise_state == IDLE) {
                throttle = 0;
            }

            // Should we be outside where cruise_state != IDLE
            // then we should not do anything with throttle here
            // it will be handled in the cruise_control.
        }

        xQueuePeek(xQueueCruiseControl, &cruise_control_button, ( TickType_t ) 0);
        xQueuePeek(xQueueGasPedal, &gas_pedal, ( TickType_t ) 0);   
        xQueuePeek(xQueueVelocity, &velocity, ( TickType_t ) 0);     
        xQueuePeek(xQueueBrakePedal, &brake_pedal, ( TickType_t ) 0);
        
        xQueueOverwrite(xQueueThrottle, &throttle);

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/**
 * =======================================================================
 * vWatchdogTask(void *args):
 *      @brief Watchdog checks every period if OK signal has been issued.
 *             As long as OK is TRUE then OVERLOAD DETECTION function has 
 *             had chance to execute and no overload has occured.
 *             If OK is FALSE then WATCHDOG initiates the ALARM and enters
 *             OVERLOADSTATE = TRUE.
 *      @param args corresponds to period of task (1000ms).
 */
void vWatchdogTask(void *args) {
    
    const TickType_t xPeriod = (uint32_t) args;
    TickType_t xLastWakeTime = 0;
    bool overloadState = false;
    bool OK;

    while(true) {

        xQueuePeek(xQueueOverloadDetected, &OK, ( TickType_t ) 0);

        if(OK == true) {
            // Do nothing. 
            // Watchdog will now be reset until next period.
            printf("Watchdog pass.\n");

            // Also reset OK signal back to 0.
            // Otherwise it will remain 1 even if overload detection sets it.
            OK = false;
            xQueueOverwrite(xQueueOverloadDetected, &OK);
        } 
        else {
            // OK was never set to 1 meaning overload.
            printf("--- SYSTEM OVERLOAD ---\n");
            BSP_SetLED(LED_GREEN, 1);
            BSP_SetLED(LED_YELLOW, 1);
            BSP_SetLED(LED_RED, 1);
            overloadState = true;
            xQueueOverwrite(xQueueOverloadState, &overloadState);
        }

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/**
 * =======================================================================
 * vOverloadTask(void *args):
 *      @brief Overload Detection Task is lowest priority task that will only
 *             be given exec time if all other tasks are finished. 
 *             So if task execs then there is no overlaod so set OK signal TRUE.
 *      @param args corresponds to period of task (1000ms).
 */
void vOverloadDetectionTask(void *args) {

    const TickType_t xPeriod = (uint32_t) args;
    TickType_t xLastWakeTime = 0;
    bool OK, overloadState;

    while(true) {
        // Function has been called so set OK signal.
        OK = true;
        xQueueOverwrite(xQueueOverloadDetected, &OK);
        printf("OverloadTask has been called.\n");

        xQueuePeek(xQueueOverloadState, &overloadState, ( TickType_t ) 0);
        if (overloadState == true) {
            // Watchdog overload alarm occured
            // Now deactivate alarm...
            BSP_SetLED(LED_GREEN, 0);
            BSP_SetLED(LED_YELLOW, 0);
            BSP_SetLED(LED_RED, 0);
            overloadState = false;
            xQueueOverwrite(xQueueOverloadState, &overloadState);
        }
       
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

void vExtraLoadTask(void *args) {

    const TickType_t xPeriod = (uint32_t) args;
    TickType_t xLastWakeTime = 0;
    uint8_t load;

    while(true) {
        
        // We cannot expect go get switch input from "button" task
        // because once overload occurs "button" task cannot run
        // and so we can never stop the overload...
        load = 0;
        load = ( (BSP_GetInput(SW_10) << 7)
               | (BSP_GetInput(SW_11) << 6)
                | (BSP_GetInput(SW_12) << 5)
                | (BSP_GetInput(SW_13) << 4)
                | (BSP_GetInput(SW_14) << 3)
                | (BSP_GetInput(SW_15) << 2)
                | (BSP_GetInput(SW_16) << 1)
                | (BSP_GetInput(SW_17)));

        
        TickType_t start = xTaskGetTickCount();
        TickType_t ticks = pdMS_TO_TICKS(load / 10);
        //printf("Extra Load: %d\n", load / 10);

        // Busy Wait for amount (load / 10) (ms).
        // Wait by converting (ms) to ticks and loop.
        while((xTaskGetTickCount() - start) < ticks);
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

// Alarm function called by watchdog. 
// Enters OVERLOADSTATE and turns on LEDs
void vTimerCallback(TimerHandle_t timer) {

    bool overloadState = true;

    printf("--- SYSTEM OVERLOAD ---\n");
    BSP_SetLED(LED_GREEN, 1);
    BSP_SetLED(LED_YELLOW, 1);
    BSP_SetLED(LED_RED, 1);
    xQueueOverwrite(xQueueOverloadState, &overloadState);
    
}

// Resets the Watchdog Timer back to 1000.
// The task is made into a background task that always runs if nothing
// else is available by simply making it a straight inifinite loop.
// Because when there are higher prio tasks available scheduler will preempt.
void vOverloadDetectionTimer(void *args) {
    bool overloadState;

    while(true) {
        //printf("RESET THE WATCHDOG TIMER.\n");
        xTimerReset(xWatchdogTimer, portMAX_DELAY);

        // Watchdog overload alarm occured. Now deactivate alarm...
        xQueuePeek(xQueueOverloadState, &overloadState, ( TickType_t ) 0);
        if (overloadState == true) {
            BSP_SetLED(LED_GREEN, 0);
            BSP_SetLED(LED_YELLOW, 0);
            BSP_SetLED(LED_RED, 0);
            overloadState = false;
            xQueueOverwrite(xQueueOverloadState, &overloadState);
        }

        vTaskDelay(10); // Small delay to not completely spam...
    }
}


/**
 * @brief The function returns the new position depending on the input parameters.
 * 
 * ==> DO NOT CHANGE THIS FUNCTION !!!
 * 
 * @param position 
 * @param velocity 
 * @param acceleration 
 * @param time_interval 
 * @return 
 */
uint16_t adjust_position(uint16_t position, int16_t velocity,
                         int8_t acceleration, uint16_t time_interval)
{
  int16_t new_position = position + velocity * time_interval / 1000
    + acceleration / 2  * (time_interval / 1000) * (time_interval / 1000);

  if (new_position > 24000) {
    new_position -= 24000;
  } else if (new_position < 0){
    new_position += 24000;
  }

  return new_position;
}


/**
 * @brief The function returns the new velocity depending on the input parameters.
 * 
 * ==> DO NOT CHANGE THIS FUNCTION !!! 
 *
 * @param velocity 
 * @param acceleration 
 * @param brake_pedal 
 * @param time_interval 
 * @return 
 */
int16_t adjust_velocity(int16_t velocity, int8_t acceleration,  
		       bool brake_pedal, uint16_t time_interval)
{
  int16_t new_velocity;
  uint8_t brake_retardation = 50;

  if (brake_pedal == false) {
    // Had to manually change here because it was casted to float after division...
    new_velocity = velocity  + ((float) (acceleration * time_interval) / 1000);
    //printf("nv: %d, v: %d, a: %d, time_int: %d\n", new_velocity, velocity, acceleration, time_interval);
    if (new_velocity <= 0) {
        new_velocity = 0;
    }
  } 
  else { 
    if ((float) (brake_retardation * time_interval) / 1000 > velocity) {
       new_velocity = 0;
    }
    else {
      new_velocity = velocity - (float) brake_retardation * time_interval / 1000;
    }
  } 

  return new_velocity;
}

/**
 * @brief The vehicle task continuously calculates the velocity of the vehicle 
 *
 * ==> DO NOT CHANGE THIS TASK !!!  
 *
 * @param args 
 */
void vVehicleTask(void *args) {
    TickType_t xLastWakeTime = 0;
    const TickType_t xPeriod = (int)args;   /* Get period (in ticks) from argument. */
    uint16_t throttle;
    bool brake_pedal;
                           /* Approximate values*/
    //=========================================
    // Changed to signed int8_t.
    int8_t acceleration;  /* Value between 40 and -20 (4.0 m/s^2 and -2.0 m/s^2) */
    uint8_t retardation;   /* Value between 20 and -10 (2.0 m/s^2 and -1.0 m/s^2) */
    uint16_t position = 0; /* Value between 0 and 24000 (0.0 m and 2400.0 m)  */
    uint16_t velocity = 0; /* Value between -200 and 700 (-20.0 m/s amd 70.0 m/s) */
    uint16_t wind_factor;   /* Value between -10 and 20 (2.0 m/s^2 and -1.0 m/s^2) */

    for (;;) {
        xQueuePeek(xQueueThrottle, &throttle, ( TickType_t ) 0);
        xQueuePeek(xQueueBrakePedal, &brake_pedal, ( TickType_t ) 0);

        /* Retardation : Factor of Terrain and Wind Resistance */
        if (velocity > 0)
	        wind_factor = velocity * velocity / 10000 + 1;
        else 
	        wind_factor = (-1) * velocity * velocity / 10000 + 1;

        if (position < 4000) 
            retardation = wind_factor; // even ground
        else if (position < 8000)
            retardation = wind_factor + 8; // traveling uphill
        else if (position < 12000)
            retardation = wind_factor + 16; // traveling steep uphill
        else if (position < 16000)
            retardation = wind_factor; // even ground
        else if (position < 20000)
            retardation = wind_factor - 8; //traveling downhill
        else
            retardation = wind_factor - 16 ; // traveling steep downhill

        acceleration = throttle / 2 - retardation;	  
        // printf("acceleration %d, retard: %d\n", acceleration, retardation);
        position = adjust_position(position, velocity, acceleration, xPeriod); 
        velocity = adjust_velocity(velocity, acceleration, brake_pedal, xPeriod);         

 
        xQueueOverwrite(xQueueVelocity, &velocity);
        xQueueOverwrite(xQueuePosition, &position); 
        vTaskDelayUntil(&xLastWakeTime, xPeriod);   /* Wait for the next release. */
    }
}

/**
 * =======================================================================
 * vDisplayTask(void *args):
 *      @brief The display task shall show the information on 
 *             - the throttle and velocity on the seven segment display
 *             - the position on the 24 LEDs (Shift registers)
 *      @param args corresponds to task period (500ms).
 */
void vDisplayTask(void *args) {
    TickType_t xLastWakeTime = 0;
    const TickType_t xPeriod = (uint32_t) args;   

    uint16_t velocity; 
    uint16_t throttle;  
    uint16_t position;
    char display7Seg[5]; // 4 chars + null byte
    uint32_t LED24 = 0;
    uint8_t *p_LED24 = (uint8_t*)&LED24;
    uint8_t step;

    // Initially clear and set brightness (0-15)
    BSP_7SegClear();
    BSP_7SegBrightness(7);

    for (;;) {
        xQueuePeek(xQueueVelocity, &velocity, ( TickType_t ) 0);
        xQueuePeek(xQueuePosition, &position, ( TickType_t ) 0);
        xQueuePeek(xQueueThrottle, &throttle, ( TickType_t ) 0);

        printf("Throttle: %d\n", throttle);
        printf("Velocity: %d\n", velocity);
        printf("Position: %d\n", position);

        // we shift the 1 depending on how many steps in mod 24 
        // that vehicle (position) has taken.
        // p_LED24 actually points to uint32_t but we only use 
        // the first 24 bits. So it behaves like uint8_t LED24[3].
        step = (position / 1000) % 24;        
        LED24 = 0x00000001 << step;
        BSP_ShiftRegWriteAll(p_LED24);

        // THROTTLE is placed in U14, U15 and VELOCITY in U16, U17
        // display7Seg = ((throttle << 16) | velocity);
        //display7Seg[0] = '0';
        //display7Seg[1] = '1';
        //display7Seg[2] = '2';
        //display7Seg[3] = '3';
        sprintf(display7Seg, "%02d%02d", throttle, velocity, sizeof(display7Seg));
        BSP_7SegDispString(display7Seg);
        
        vTaskDelayUntil(&xLastWakeTime, xPeriod);   /* Wait for the next release. */
    }
}

/**
 * @brief Main program that starts all the tasks and the scheduler
 * 
 * ==> MODIFY THE MAIN PROGRAM!
 *        - Convert the button and control tasks to periodic tasks
 *        - Adjust the priorities of the task so that they correspond
 *          to the rate-monotonic algorithm.
 * @return 
 */
int main()
{
    BSP_Init();  /* Initialize all components on the ES Lab-Kit. */

    // ================================================================================
    //          Task            Name        STACK   PERIOD      PRIO        POINTER
    xTaskCreate(vButtonTask, "Button Task",   512, (void*) 50,      5, &xButton_handle);
    xTaskCreate(vVehicleTask, "Vehicle Task", 512, (void*) 100,     4, &xVehicle_handle); 
    xTaskCreate(vControlTask, "Control Task", 512, (void*) 200,     3, &xControl_handle);
    xTaskCreate(vDisplayTask, "Display Task", 512, (void*) 500,     2, &xDisplay_handle); 

    xTaskCreate(vWatchdogTask, "Watchdog Task", 512, (void*) 1000,  7, &xWatchdog_handle);
    xTaskCreate(vOverloadDetectionTask, "OverloadDetection Task",  512, (void*) 1000,  1, &xOverloadDetection_handle);
    xTaskCreate(vExtraLoadTask, "ExtraLoad Task",  512, (void*) 25,  6, &xExtraLoad_handle);
    
    /* For Watchdog Timer (conditional A)*/
    xTaskCreate(vOverloadDetectionTimer, "Overload Task",  512, (void*) 1000,  1, &xOverloadDetection_handle);

    /* Create the message queues */
    xQueueCruiseControl = xQueueCreate( 1, sizeof(bool));
    xQueueGasPedal      = xQueueCreate( 1, sizeof(bool));
    xQueueBrakePedal    = xQueueCreate( 1, sizeof(bool));
    xQueueVelocity      = xQueueCreate( 1, sizeof(uint16_t));
    xQueuePosition      = xQueueCreate( 1, sizeof(uint16_t));
    xQueueThrottle      = xQueueCreate( 1, sizeof(uint16_t));

    xQueueOverloadDetected = xQueueCreate( 1, sizeof(bool));
    xQueueOverloadState = xQueueCreate( 1, sizeof(bool));
    xQueueSwitches      = xQueueCreate( 1, sizeof(uint8_t));

    xWatchdogTimer = xTimerCreate("Watchdog Timer", pdMS_TO_TICKS(1000), pdFALSE, 0, vTimerCallback);
    xTimerStart(xWatchdogTimer, portMAX_DELAY);    

    vTaskStartScheduler();  /* Start the scheduler. */
    
    return 0;
}
/*-----------------------------------------------------------*/
