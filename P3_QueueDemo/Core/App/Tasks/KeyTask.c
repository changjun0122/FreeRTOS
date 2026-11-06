#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "Types/LedTypes.h"

#include <stdlib.h> 

#define IS_KEY_PRESSED() (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_RESET)

#define KEY_CHECK_INTERVAL  10

#define KEY_DEBOUNCE_TIME  30

#define KEY_DEBOUNCE_COUNT  (KEY_DEBOUNCE_TIME / KEY_CHECK_INTERVAL)

uint8_t isKey1Clicked()
{
    static uint8_t count = 0;
    static uint8_t pressed = 0;
    if(IS_KEY_PRESSED() && ! pressed)
    {
        count++;
        if(count >= KEY_DEBOUNCE_COUNT && IS_KEY_PRESSED() )
        {
            pressed = 1;
            return 1;
        }
    }
    if(!IS_KEY_PRESSED())
    {
        pressed = 0;
        count = 0;
    }
    return 0;
}

typedef enum
{
    LEDColor_RED = 0,
    LEDColor_GREEN = 1,
    LEDColor_BLUE = 2,
} LEDColor;

typedef enum
{
    LEDState_OFF = 0,
    LEDState_ON = 1,
} LEDState;

typedef struct
{
    LEDColor color;
    LEDState state;
} LEDMessage;

void StartKeyTask(void *argument)
{
    LEDState ledState = LEDState_OFF;
    for (;;) {
        if(isKey1Clicked())
        {
            ledState = !ledState;
            LEDMessage* message = pvPortMalloc(sizeof(LEDMessage));
            message->color = LEDColor_RED;
            message->state = ledState;
            osMessageQueuePut(LEDQueueHandle, &message, NULL, osWaitForever);
        }
        osDelay(10);
    }
}
