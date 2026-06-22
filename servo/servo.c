#include "servo.h"
#include "delay.h"

/*
 * 初始化舵机总线 UART (USART2, 半双工)
 * PA2=TX, PA3=RX (外接 1kΩ 电阻实现半双工单线，或接 Yeahbot)
 */
void servo_init(uint32_t baudrate)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // PA2 = TX (推挽复用输出)
    gpio.GPIO_Pin   = GPIO_Pin_2;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // PA3 = RX (浮空输入)
    gpio.GPIO_Pin  = GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    usart.USART_BaudRate            = baudrate;
    usart.USART_WordLength          = USART_WordLength_8b;
    usart.USART_StopBits            = USART_StopBits_1;
    usart.USART_Parity              = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &usart);

    USART_HalfDuplexCmd(USART2, ENABLE);  // 半双工单线模式
    USART_Cmd(USART2, ENABLE);
}

/*
 * 将多舵机控制帧拆为单舵机帧，逐帧发送
 *
 * 输入(来自NUC, 9600bps):
 *   0x55 0x55 Len 0x03 N Time_L Time_H [ID Pos_L Pos_H]×N
 *
 * 输出(到LX-16A总线, 115200bps):
 *   每个舵机一帧: 0x55 0x55 ID 0x07 0x01 Pos_L Pos_H Time_L Time_H CS
 */
void servo_forward_frame(uint8_t *frame, uint8_t len)
{
    uint8_t  i, j, n, id, *p;
    uint16_t pos, time;
    uint8_t out[10];   // 加校验和，从 9 改为 10
    uint8_t checksum;

    if (len < 7 || frame[3] != 0x03) return;

    n    = frame[4];
    time = frame[5] | (frame[6] << 8);
    p    = frame + 7;

    for (i = 0; i < n; i++) {
        id  = p[0];
        pos = p[1] | (p[2] << 8);

        out[0] = 0x55;
        out[1] = 0x55;
        out[2] = id;
        out[3] = 0x07;
        out[4] = 0x01;        // SERIAL_SERVO_MOVE_TIME_WRITE
        out[5] = pos & 0xFF;
        out[6] = (pos >> 8) & 0xFF;
        out[7] = time & 0xFF;
        out[8] = (time >> 8) & 0xFF;

        // 校验和: 对 out[2]~out[8] 累加后取反
        checksum = 0;
        for (j = 2; j < 9; j++) {
            checksum += out[j];
        }
        out[9] = ~checksum;

        for (j = 0; j < 10; j++) {
            USART_SendData(USART2, out[j]);
            while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
        }

        delay_us(500);
        p += 3;
    }
}
