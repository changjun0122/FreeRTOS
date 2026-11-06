#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "Types/LEDType.h"

#include <stdlib.h>
#include <string.h>

#include "usart.h"

#define IS_KEY_PRESSED() (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_RESET)

#define KEY_CHECK_INTERVAL  10

#define KEY_DEBOUNCE_TIME  30

#define KEY_DEBOUNCE_COUNT  (KEY_DEBOUNCE_TIME / KEY_CHECK_INTERVAL)

uint8_t isKey1Clicked()
{
    static uint8_t count = 0;
    static uint8_t pressed = 0;
    if(IS_KEY_PRESSED())
    {
        osDelay(KEY_DEBOUNCE_TIME);
        if(IS_KEY_PRESSED())
        {
            return 1;
        }
    }
    return 0;
}

void StartKeyTask(void *argument)
{
    LEDState ledState = LEDState_OFF;
    char message[64] = "获取到的信号量";
    for (;;) {
        osSemaphoreAcquire(KeySemaphoreHandle, osWaitForever);
        HAL_UART_Transmit(&huart2, (uint8_t *)message, strlen(message), 1000);
        if(isKey1Clicked())
        {
            ledState = !ledState;
            LEDMessage *message = pvPortMalloc(sizeof(LEDMessage));
            if (message != NULL) {
                message->color = LEDColor_RED;
                message->state = ledState;
                osMessageQueuePut(LEDQueueHandle, &message, 0, osWaitForever);
            }
        }
        osDelay(10);
    }
}
