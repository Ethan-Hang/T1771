#ifndef __UART_PARSE_TASK_H__
#define __UART_PARSE_TASK_H__

#include <stdint.h>

#define FRAME_NOT_DETECTED (0x01) //Status of NOT detected
#define FRAME_HEAD         (0x02) //Status of HEAD detected
#define FRAME_END          (0x03) //Status of END  detected


#define FRAME_HEAD_FLAG    (0xFE) //FRAME_HEAD pattern
#define FRAME_END_FLAG     (0xFF) //FRAME_END  pattern

// 声明全局变量（使用 extern）
extern uint8_t g_buffer1[1];
extern uint8_t g_buffer2[1];

// 声明函数
void uart_rec_A_func(void *argument);

#endif
