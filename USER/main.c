#include "stm32f10x.h"
#include "delay.h"
#include "uart.h"
#include "servo.h"

static uint8_t rx_buf[64];
static uint8_t rx_index = 0;
static uint8_t rx_state = 0;
static uint8_t got_data = 0;  // 收到数据标记

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE)) {
        uint8_t byte = USART_ReceiveData(USART1);
        got_data = 1;  // 标记收到数据

        switch (rx_state) {
        case 0:
            if (byte == 0x55) { rx_state = 1; rx_buf[0] = 0x55; }
            break;
        case 1:
            if (byte == 0x55) { rx_state = 2; rx_buf[1] = 0x55; rx_index = 2; }
            else { rx_state = 0; }
            break;
        case 2:
            rx_buf[rx_index++] = byte;
            if (rx_index >= 2 && rx_index >= rx_buf[2] + 2) {
                servo_forward_frame(rx_buf, rx_index);
                rx_state = 0;
            }
            break;
        }
    }
}

int main(void)
{
    int k;
    NVIC_InitTypeDef nvic;
    GPIO_InitTypeDef g;

    SystemInit();
    delay_init(72);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    // LED (PC13)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    g.GPIO_Pin = GPIO_Pin_13;
    g.GPIO_Mode = GPIO_Mode_Out_PP;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &g);
    GPIO_SetBits(GPIOC, GPIO_Pin_13);

    // 启动闪 3 次
    for (k = 0; k < 3; k++) {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13); delay_ms(200);
        GPIO_SetBits(GPIOC, GPIO_Pin_13);   delay_ms(200);
    }

    uart1_init(9600);
    servo_init(115200);

    // USART1 中断
    nvic.NVIC_IRQChannel = USART1_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    while (1) {
        // 收到任何字节 LED 亮 500ms
        if (got_data) {
            got_data = 0;
            GPIO_ResetBits(GPIOC, GPIO_Pin_13);
            delay_ms(500);
            GPIO_SetBits(GPIOC, GPIO_Pin_13);
        }
    }
}
