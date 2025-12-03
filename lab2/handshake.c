#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "bsp.h"

const uint8_t redLED_seq[4]   = {1, 1, 0, 0};
const uint8_t greenLED_seq[4] = {1, 0, 0, 1};

volatile uint8_t stepIndex = 0; 
SemaphoreHandle_t xMutex;

/*************************************************************/

void vTaskRed(void *pvParameters) {
    while(1) {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        uint8_t state = redLED_seq[stepIndex];
        xSemaphoreGive(xMutex);
        
        if(state) BSP_SetLED(LED_RED,1);
        else BSP_SetLED(LED_RED,0);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vTaskGreen(void *pvParameters) {
    while(1) {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        uint8_t state = greenLED_seq[stepIndex];
        stepIndex = (stepIndex + 1) % 4; 
        xSemaphoreGive(xMutex);
        
        if(state) BSP_SetLED(LED_GREEN,1);
        else BSP_SetLED(LED_GREEN,0);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/**
 * @brief Main function.
 * 
 * @return int 
 */
int main()
{
    BSP_Init();             /* Initialize all components on the lab-kit. */
    
    xMutex = xSemaphoreCreateMutex();

    /* Create the tasks. */
     xTaskCreate(vTaskRed, "Red Task", 128, NULL, 1, NULL);
     xTaskCreate(vTaskGreen, "Green Task", 128, NULL, 1, NULL);

    
    vTaskStartScheduler();  /* Start the scheduler. */
    
    while (true) { 
     /* Should not reach here... */
    }
}
/*-----------------------------------------------------------*/
