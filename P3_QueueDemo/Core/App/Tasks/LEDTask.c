#include "cmsis_os2.h"
#include "main.h"
#include "Types/LedTypes.h"
#include "FreeRTOS.h"

void StartLEDTask(void *argument) {
    for (;;) {
        LEDMessage *message;
        osMessageQueueGet(LEDQueueHandle, &message, NULL, osWaitForever);
        switch(message->color)
        {
        case LEDColor_RED:
            HAL_GPIO_WritePin(RedLED_GPIO_GPIO_Port, RedLED_GPIO_Pin, message->state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
        case LEDColor_GREEN:
            HAL_GPIO_WritePin(GreenLED_GPIO_GPIO_Port, GreenLED_GPIO_Pin, message->state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
        case LEDColor_BLUE:
            HAL_GPIO_WritePin(BlueLED_GPIO_GPIO_Port, BlueLED_GPIO_Pin, message->state ? GPIO_PIN_SET : GPIO_PIN_RESET);  
            break;
        default:
            break;
        }
        vPortFree(message);
    }
}