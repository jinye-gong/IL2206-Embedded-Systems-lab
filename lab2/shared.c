#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "bsp.h"

TaskHandle_t    A, B; 
SemaphoreHandle_t semA, semB;
BaseType_t sharedAddress;

/**
 * @brief Blink task.
 * @param args Task period (uint32_t).
 */
void task_A(void *args);
void task_B(void *args);

/*************************************************************/

/**
 * @brief Main function.
 * In case Serial Port monitor does not work
 * try in terminal:
 * screen /dev/ttyACM0 115200
 * @return int 
 */
int main()
{
    BSP_Init();             

    semA = xSemaphoreCreateCounting(1,1);
    semB = xSemaphoreCreateCounting(1,0);

    xTaskCreate(task_A, "Task A", 512, (void*) 1000, 2, &A);
    xTaskCreate(task_B, "Task B", 512, (void*) 1000, 2, &B);
    
    vTaskStartScheduler();  /* Start the scheduler. */
    
    while (true) { 
        sleep_ms(1000); /* Should not reach here... */
    }
}
/*-----------------------------------------------------------*/

void task_A(void *args) {
    TickType_t xLastWakeTime = 0;
    const TickType_t xPeriod = (int)args;   /* Get period (in ticks) from argument. */

    BaseType_t counter = 1;

    for (;;) {

        /* Write to sharedAddress */
        sharedAddress = counter;
        printf("Sending : %d\n", counter);
        counter++;

        /* Signal_semB() */
        xSemaphoreGive(semB);

        /* wait_semA() with timeout of exceeds xPeriod */
        xSemaphoreTake(semA, xPeriod);

        /* Read sharedAddress */
        BaseType_t received = sharedAddress;
        printf("Receiving : %d\n", received);


        vTaskDelayUntil(&xLastWakeTime, xPeriod);   
    }
}

void task_B(void *args) {
    TickType_t xLastWakeTime = 0;
    const TickType_t xPeriod = (int)args;   /* Get period (in ticks) from argument. */

    for (;;) {
        /* wait_semB() with timeout of exceeds xPeriod */
        xSemaphoreTake(semB, xPeriod);

        /* Convert number and write to sharedAddress */
        sharedAddress = sharedAddress * (-1);
        
        /* Signal_semA() */
        xSemaphoreGive(semA);

        vTaskDelayUntil(&xLastWakeTime, xPeriod);   /* Wait for the next release. */
    }
}
/*-----------------------------------------------------------*/
