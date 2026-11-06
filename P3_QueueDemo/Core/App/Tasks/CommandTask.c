#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "main.h"
#include "usart.h"
#include "Types/LEDType.h"

#define COMMAND_HEADER      0xAA
#define COMMAND_MAX_LENGTH  8

void StartCommandTask(void *argument)
{
    uint8_t receive;
    uint8_t command[COMMAND_MAX_LENGTH];
    uint8_t commandIndex = 0;
    uint8_t commandLength = 0;

    UART2_Receive_Start();

    for (;;) {
        if (osMessageQueueGet(CommandQueueHandle, &receive, NULL, osWaitForever) != osOK) {
            continue;
        }

        /* 阶段 1：等待帧头 */
        if (commandIndex == 0) {
            if (receive == COMMAND_HEADER) {
                command[commandIndex++] = receive;
            }
            continue;
        }

        /* 阶段 2：长度字节 */
        if (commandIndex == 1) {
            if (receive < 4 || receive > COMMAND_MAX_LENGTH) {
                commandIndex = 0;
                continue;
            }
            commandLength = receive;
            command[commandIndex++] = receive;
            continue;
        }

        /* 阶段 3：收正文 */
        command[commandIndex++] = receive;
        if (commandIndex < commandLength) {
            continue;
        }

        /* 一帧收齐：校验和 = 除最后一字节外，其余所有字节之和 */
        uint8_t checksum = 0;
        for (uint8_t i = 0; i < commandLength - 1; i++) {
            checksum += command[i];
        }

        if (checksum == command[commandLength - 1]) {
            /* 正文从下标 2 开始，每两个字节是一对 (颜色, 状态) */
            for (uint8_t i = 2; i < commandLength - 2; i += 2) {
                LEDMessage *message = pvPortMalloc(sizeof(LEDMessage));
                if (message == NULL) {
                    continue;
                }
                message->color = (LEDColor)(command[i] - 1);
                message->state = (LEDState)command[i + 1];
                osMessageQueuePut(LEDQueueHandle, &message, 0, osWaitForever);
            }
        }

        commandIndex = 0;
        commandLength = 0;
    }
}
