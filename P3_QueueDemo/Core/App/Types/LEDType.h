//
// Created by 35231 on 2026/11/6.
//

#ifndef P3_QUEUEDEMO_LEDTYPE_H
#define P3_QUEUEDEMO_LEDTYPE_H

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


#endif //P3_QUEUEDEMO_LEDTYPE_H
